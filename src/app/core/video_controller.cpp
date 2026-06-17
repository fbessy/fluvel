// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_controller.hpp"
#include "camera_format_utils.hpp"
#include "contour_adapters.hpp"
#include "frame_clock.hpp"
#include "streaming_stats.hpp"

#include <QAudioOutput>
#include <QCamera>
#include <QFileInfo>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QUrl>
#include <QVideoSink>

#include <cassert>
#include <utility>

namespace fluvel
{

VideoController::VideoController(const VideoSessionSettings& session, QObject* parent)
    : QObject(parent)
    , activeContourThread_(this)
{
    auto mediaDevices = new QMediaDevices(this);

    connect(mediaDevices, &QMediaDevices::videoInputsChanged, this,
            &VideoController::onVideoInputsChanged);

    onVideoSettingsChanged(session);
    onVideoDisplaySettingsChanged(session.display);

    connect(&activeContourThread_, &VideoActiveContourThread::displayFrameReady, this,
            &VideoController::onDisplayFrameReady, Qt::QueuedConnection);

    activeContourThread_.start();

    startupTimer_.setSingleShot(true);
    watchdogTimer_.setInterval(kWatchdogPeriodMs);
    diagnosticsTimer_.setInterval(kDiagnosticsPeriodMs);

    connect(&startupTimer_, &QTimer::timeout, this, &VideoController::onStartupTimeout);
    connect(&watchdogTimer_, &QTimer::timeout, this, &VideoController::checkWatchdog);
    connect(&diagnosticsTimer_, &QTimer::timeout, this, &VideoController::updateDiagnostics);

    connect(&activeContourThread_, &VideoActiveContourThread::frameProcessed, this,
            &VideoController::onFrameProcessed);

    connect(&mediaPlayer_, &QMediaPlayer::errorOccurred, this,
            &VideoController::onMediaPlayerError);

    connect(&mediaPlayer_, &QMediaPlayer::mediaStatusChanged, this,
            &VideoController::onMediaStatusChanged);

    connect(&videoSink_, &QVideoSink::videoFrameChanged, this, &VideoController::onFrameReceived);

    connect(&mediaPlayer_, &QMediaPlayer::positionChanged, this,
            &VideoController::playbackPositionChanged);

    connect(&mediaPlayer_, &QMediaPlayer::metaDataChanged, this,
            &VideoController::onMetaDataChanged);

    audioOutput_.setVolume(0.5f);
    mediaPlayer_.setAudioOutput(&audioOutput_);
}

VideoController::~VideoController()
{
    stop();
    activeContourThread_.stop();
    activeContourThread_.wait();
}

void VideoController::start(const SourceConfig& sourceConfig)
{
    switch (sourceConfig.type)
    {
        case SourceType::Camera:
            start(sourceConfig.cameraId, sourceConfig.cameraFormat);
            return;

        case SourceType::Media:
            start(sourceConfig.url);
            return;

        case SourceType::None:
            return;
    }

    std::unreachable();
}

void VideoController::start(const QByteArray& deviceId)
{
    start(deviceId, QCameraFormat());
}

void VideoController::start(const QByteArray& deviceId, const QCameraFormat& format)
{
    if (state_ != StreamingState::Stopped || camera_)
        return;

    state_ = StreamingState::Starting;
    emit streamingStarting();

    const auto cameras = QMediaDevices::videoInputs();
    bool isFound = false;

    for (const auto& cam : cameras)
    {
        if (cam.id() == deviceId)
        {
            isFound = true;

            camera_ = new QCamera(cam, this);

            // 👉 Application du format choisi (UI)
            if (!format.isNull())
            {
                const auto formats = cam.videoFormats();

                auto it = std::find_if(formats.begin(), formats.end(),
                                       [&](const QCameraFormat& f)
                                       {
                                           return camera_utils::isSameCameraFormat(f, format);
                                       });

                if (it != formats.end())
                    camera_->setCameraFormat(*it);
            }

            startupInfo_ = {};
            startupInfo_.type = SourceType::Camera;
            startupInfo_.deviceId = camera_->cameraDevice().id();
            startupInfo_.deviceFormat = camera_->cameraFormat();
            startupInfo_.description = camera_->cameraDevice().description();

            mediaInfo_ = {};

            captureSession_.setCamera(camera_);
            captureSession_.setVideoSink(&videoSink_);

            connect(camera_, &QCamera::errorOccurred, this, &VideoController::onCameraError);

#ifdef FLUVEL_SIMULATE_STREAM_LOSS
            testFrameCounter_ = 0;
#endif

            startupTimer_.start(kStartupTimeoutMs);

            camera_->start();

            break;
        }
    }

    if (!isFound)
    {
        CameraErrorInfo err;

        err.error = QCamera::NoError; // not a backend error
        err.errorString = tr("Camera not found");
        err.state = StreamingState::Starting;

        err.sourceInfo.type = SourceType::Camera;
        err.sourceInfo.deviceId = deviceId;

        emit cameraError(err);
    }
}

void VideoController::start(const QUrl& url)
{
    if (state_ != StreamingState::Stopped || camera_)
        return;

    state_ = StreamingState::Starting;
    emit streamingStarting();

    startupInfo_ = {};
    startupInfo_.type = SourceType::Media;
    startupInfo_.sourceUrl = url;

    mediaInfo_ = {};

    mediaPlayer_.setSource(url);
    mediaPlayer_.setVideoSink(&videoSink_);

#ifdef FLUVEL_SIMULATE_STREAM_LOSS
    testFrameCounter_ = 0;
#endif

    startupTimer_.start(kStartupTimeoutMs);

    mediaPlayer_.play();
}

void VideoController::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    updateMediaInfo();

    if (status == QMediaPlayer::EndOfMedia)
    {
        stop();
    }
}

void VideoController::stop()
{
    if (state_ == StreamingState::Stopped)
        return;

    startupTimer_.stop();
    watchdogTimer_.stop();
    diagnosticsTimer_.stop();

    if (camera_)
    {
        disconnect(camera_, &QCamera::errorOccurred, this, &VideoController::onCameraError);
        camera_->stop();
    }

    captureSession_.setVideoSink(nullptr);
    captureSession_.setCamera(nullptr);

    mediaPlayer_.stop();

    if (camera_)
    {
        delete camera_;
        camera_ = nullptr;
    }

    mediaInfo_ = {};

    state_ = StreamingState::Stopped;
    emit streamingStopped();
}

void VideoController::onFrameReceived(const QVideoFrame& frame)
{
#ifdef FLUVEL_SIMULATE_STARTUP_TIMEOUT
    return;
#endif

    if (!frame.isValid())
        return;

#ifdef FLUVEL_SIMULATE_STREAM_LOSS
    if (++testFrameCounter_ > 90) // after ~3 seconds if 30 fps
        return;
#endif

    const int64_t now = FrameClock::nowNs();
    lastValidFrameTsNs_ = now;
    watchdogArmed_ = true;

    if (state_ == StreamingState::Starting)
    {
        state_ = StreamingState::Streaming;

        startupTimer_.stop();
        watchdogTimer_.start();

        frameStats_.reset();
        diagnosticsTimer_.start();

        streamingInfo_ = {};
        streamingInfo_.source = startupInfo_;

        streamingInfo_.frameSize = frame.size();
        streamingInfo_.pixelFormat = frame.pixelFormat();
        streamingInfo_.sourceFrameRate = frame.streamFrameRate();

        emit streamingStarted(streamingInfo_);
    }

    frameStats_.frameReceived();

    ReceivedFrame cf;
    cf.frame = frame;
    cf.receiveTimestampNs = now;

    activeContourThread_.submitFrame(cf);
}

void VideoController::onFrameProcessed(quint64 contourSize)
{
    frameStats_.frameProcessed(contourSize);
}

void VideoController::onFrameDisplayed(const FrameTimestamps& ts)
{
    frameStats_.frameDisplayed(ts);
}

void VideoController::onDisplayFrameReady(const DisplayFrame& displayFrame)
{
    UiFrame uiDisplayFrame;

    uiDisplayFrame.image = displayFrame.image;

    if (uiDisplayFrame.image.isNull())
        return;

    uiDisplayFrame.outerContour = convertToQVector(displayFrame.outerContour);
    uiDisplayFrame.innerContour = convertToQVector(displayFrame.innerContour);

    uiDisplayFrame.receiveTimestampNs = displayFrame.receiveTimestampNs;
    uiDisplayFrame.processTimestampNs = displayFrame.processTimestampNs;

    emit imageAndContourUpdated(uiDisplayFrame);
}

void VideoController::onStartupTimeout()
{
    stop();
    emit startupTimeout(startupInfo_, static_cast<double>(kStartupTimeoutMs) / 1000.0);
}

void VideoController::checkWatchdog()
{
    if (!watchdogArmed_)
        return;

    const int64_t frameAgeNs = FrameClock::nowNs() - lastValidFrameTsNs_;

    if (frameAgeNs > kStreamLossTimeoutNs)
    {
        stop();
        emit streamingLost(streamingInfo_, static_cast<double>(frameAgeNs) / 1e9);
    }
}

void VideoController::updateDiagnostics()
{
    auto snap = frameStats_.snapshot();

    StreamingStats stats{snap.receivedFps,      snap.processedFps,        snap.displayedFps,
                         snap.dropRate,         snap.avgLatencyDisplayMs, snap.maxLatencyDisplayMs,
                         snap.avgLatencyProcMs, snap.avgContourSize};

    QString textStats = QString(tr("In | Proc | Disp: %1 | %2 | %3 fps\n"
                                   "Lat: %4 ms (proc %5) | Drop: %6 %\n"
                                   "Contour: %7 pts"))
                            .arg(stats.receivedFps, 0, 'f', 1)
                            .arg(stats.processedFps, 0, 'f', 1)
                            .arg(stats.displayedFps, 0, 'f', 1)
                            .arg(stats.avgLatencyDisplayMs, 0, 'f', 1)
                            .arg(stats.avgLatencyProcMs, 0, 'f', 1)
                            .arg(100.f * stats.dropRate, 0, 'f', 1)
                            .arg(stats.avgContourSize, 0, 'f', 0);

    emit textStatsUpdated(textStats);
}

void VideoController::onVideoInputsChanged()
{
    const auto devices = QMediaDevices::videoInputs();

    handleActiveDeviceUnplug(devices);

    emit videoInputsChanged(devices);
}

void VideoController::handleActiveDeviceUnplug(const QList<QCameraDevice>& devices)
{
    if (state_ == StreamingState::Stopped)
        return;

    const bool cameraStillExists = std::any_of(devices.begin(), devices.end(),
                                               [&](const QCameraDevice& dev)
                                               {
                                                   return dev.id() == startupInfo_.deviceId;
                                               });

    if (!cameraStillExists)
        stop();
}

void VideoController::onCameraError(QCamera::Error error, const QString& errorString)
{
    if (!camera_)
        return;

    const StreamingState state = state_;

    CameraErrorInfo camError;
    camError.error = error;
    camError.errorString = errorString;
    camError.sourceInfo = startupInfo_;
    camError.state = state;

    if (state == StreamingState::Starting)
        stop();

    emit cameraError(camError);
}

void VideoController::onMediaPlayerError(QMediaPlayer::Error error, const QString& errorString)
{
    const StreamingState state = state_;

    MediaPlayerErrorInfo mediaError;
    mediaError.error = error;
    mediaError.errorString = errorString;
    mediaError.sourceInfo = startupInfo_;
    mediaError.state = state;

    if (state == StreamingState::Starting)
        stop();

    emit mediaPlayerError(mediaError);
}

void VideoController::onVideoSettingsChanged(const VideoSessionSettings& session)
{
    activeContourThread_.setAlgoConfig(session.compute);

    emit downscaleChanged(session.compute.downscale);
}

void VideoController::onVideoDisplaySettingsChanged(const DisplayConfig& display)
{
    activeContourThread_.setDisplayMode(display.displayMode);
}

bool VideoController::isStreaming() const
{
    return state_ == StreamingState::Streaming;
}

StreamingState VideoController::streamingState() const
{
    return state_;
}

SourceInfo VideoController::activeSource() const
{
    if (state_ != StreamingState::Streaming)
        return {};

    return streamingInfo_.source;
}

QList<QCameraDevice> VideoController::videoInputs() const
{
    return QMediaDevices::videoInputs();
}

void VideoController::seek(qint64 posMs)
{
    watchdogArmed_ = false;

    mediaPlayer_.setPosition(posMs);
}

void VideoController::onMetaDataChanged()
{
    updateMediaInfo();
}

bool VideoController::isUsefulMediaTitle(const QString& title)
{
    const QString trimmed = title.trimmed();

    if (trimmed.isEmpty())
        return false;

    static const QStringList kIgnoredTitles{"video",    "track",   "track 1",
                                            "untitled", "unknown", "media"};

    return !kIgnoredTitles.contains(trimmed.toLower());
}

void VideoController::updateMediaInfo()
{
    MediaInfo info{};

    info.seekable = mediaPlayer_.isSeekable();
    info.durationMs = mediaPlayer_.duration();

    const QString title = mediaPlayer_.metaData().stringValue(QMediaMetaData::Title);

    if (isUsefulMediaTitle(title))
        info.title = title;

    const double fps = mediaPlayer_.metaData().value(QMediaMetaData::VideoFrameRate).toDouble();

    if (fps > 0.0)
        info.frameRate = fps;

    mediaInfo_ = info;

    emit mediaInfoChanged(mediaInfo_);
}

} // namespace fluvel
