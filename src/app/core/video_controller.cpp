// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_controller.hpp"
#include "contour_adapters.hpp"
#include "frame_clock.hpp"
#include "frame_rendering_utils.hpp"
#include "streaming_stats.hpp"

#include <QAudioOutput>
#include <QDir>
#include <QFileInfo>
#include <QMediaDevices>
#include <QMediaPlayer>
#include <QUrl>
#include <QVideoSink>
#include <QtNumeric>

#include <utility>

namespace fluvel
{

VideoController::VideoController(const VideoSessionSettings& session, QObject* parent)
    : QObject(parent)
    , cameraSource_(this)
    , audioOutput_(this)
    , mediaSource_(&audioOutput_, this)
    , processingThread_(this)
{
    auto mediaDevices = new QMediaDevices(this);

    onVideoSettingsChanged(session);
    onVideoDisplaySettingsChanged(session.display);

    startupTimer_.setSingleShot(true);
    watchdogTimer_.setInterval(kWatchdogPeriodMs);
    diagnosticsTimer_.setInterval(kDiagnosticsPeriodMs);

    cameraSource_.setVideoSink(&videoSink_);
    mediaSource_.setVideoSink(&videoSink_);

    connect(&cameraSource_, &CameraVideoSource::error, this, &VideoController::onCameraError);
    connect(&mediaSource_, &MediaVideoSource::error, this, &VideoController::onMediaPlayerError);

    connect(&audioOutput_, &QAudioOutput::volumeChanged, this,
            [this](float volume)
            {
                emit volumeChanged(volume);
            });

    connect(&audioOutput_, &QAudioOutput::mutedChanged, this,
            [this](bool muted)
            {
                emit mutedChanged(muted);
            });

    connect(&mediaSource_, &MediaVideoSource::mediaStatusChanged, this,
            &VideoController::onMediaStatusChanged);

    connect(&mediaSource_, &MediaVideoSource::positionChanged, this,
            &VideoController::playbackPositionChanged);

    connect(&mediaSource_, &MediaVideoSource::playbackStateChanged, this,
            [this](QMediaPlayer::PlaybackState state)
            {
                emit playbackStateChanged(state);
                emit pausedChanged(state == QMediaPlayer::PausedState);
            });

    connect(&mediaSource_, &MediaVideoSource::mediaInfoChanged, this,
            &VideoController::mediaInfoChanged);

    //
    // Devices
    //
    connect(mediaDevices, &QMediaDevices::videoInputsChanged, this,
            &VideoController::onVideoInputsChanged);

    //
    // Timers
    //
    connect(&startupTimer_, &QTimer::timeout, this, &VideoController::onStartupTimeout);
    connect(&watchdogTimer_, &QTimer::timeout, this, &VideoController::checkWatchdog);
    connect(&diagnosticsTimer_, &QTimer::timeout, this, &VideoController::updateDiagnostics);

    connect(&videoSink_, &QVideoSink::videoFrameChanged, this, &VideoController::onFrameReceived);

    //
    // Processing thread
    //
    connect(&processingThread_, &VideoProcessingThread::processedFrameReady, this,
            &VideoController::onProcessedFrameReady, Qt::QueuedConnection);

    connect(&processingThread_, &VideoProcessingThread::frameProcessed, this,
            &VideoController::onFrameProcessed);

    processingThread_.start();
}

VideoController::~VideoController()
{
    stop();
    processingThread_.stop();
    processingThread_.wait();
}

void VideoController::start(const SourceConfig& sourceConfig)
{
    switch (sourceConfig.type)
    {
        case SourceType::Camera:
            start(sourceConfig.camera.deviceId, sourceConfig.camera.deviceFormat);
            return;

        case SourceType::Media:
            start(sourceConfig.media.sourceUrl);
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
    if (state_ != StreamingState::Stopped || cameraSource_.isActive())
        return;

    state_ = StreamingState::Starting;
    emit streamingStarting();

    const CameraConfig config{.deviceId = deviceId, .deviceFormat = format};

    if (!cameraSource_.start(config))
    {
        CameraErrorInfo err;
        err.error = QCamera::NoError;
        err.errorString = tr("Camera not found");
        err.state = StreamingState::Starting;
        err.sourceInfo.type = SourceType::Camera;
        err.sourceInfo.camera.deviceId = deviceId;

        emit cameraError(err);
        return;
    }

    startupInfo_ = {};
    startupInfo_.type = SourceType::Camera;
    startupInfo_.camera = cameraSource_.cameraInfo();

#ifdef FLUVEL_SIMULATE_STREAM_LOSS
    testFrameCounter_ = 0;
#endif

    startupTimer_.start(kStartupTimeoutMs);
}

void VideoController::start(const QUrl& url)
{
    if (state_ != StreamingState::Stopped || cameraSource_.isActive())
        return;

    state_ = StreamingState::Starting;
    emit streamingStarting();

    startupInfo_ = {};
    startupInfo_.type = SourceType::Media;
    startupInfo_.media.sourceUrl = url;

    const MediaSourceConfig config{.sourceUrl = url};

#ifdef FLUVEL_SIMULATE_STREAM_LOSS
    testFrameCounter_ = 0;
#endif

    startupTimer_.start(kStartupTimeoutMs);

    if (!mediaSource_.start(config))
    {
        state_ = StreamingState::Stopped;
        return;
    }
}

void VideoController::stop()
{
    if (state_ == StreamingState::Stopped)
        return;

    watchdog_.reset();

    startupTimer_.stop();
    watchdogTimer_.stop();
    diagnosticsTimer_.stop();

    cameraSource_.stop();
    mediaSource_.stop();

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

    watchdog_.frameReceived(now);

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

    processingThread_.submitFrame(cf);
}

void VideoController::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia)
        stop();
}

void VideoController::onFrameProcessed(quint64 contourSize)
{
    frameStats_.frameProcessed(contourSize);
}

void VideoController::onFrameDisplayed(const FrameTimestamps& ts)
{
    frameStats_.frameDisplayed(ts);
}

void VideoController::onProcessedFrameReady(const ProcessedFrame& frame)
{
    DisplayFrame displayFrame;

    displayFrame.image = frame.image;

    if (displayFrame.image.isNull())
        return;

    displayFrame.outerContour = convertToQVector(frame.outerContour);
    displayFrame.innerContour = convertToQVector(frame.innerContour);

    displayFrame.receiveTimestampNs = frame.receiveTimestampNs;
    displayFrame.processTimestampNs = frame.processTimestampNs;

    lastDisplayFrame_ = displayFrame;

    emit displayFrameReady(displayFrame);
}

void VideoController::onStartupTimeout()
{
    stop();

    emit startupTimeout(startupInfo_, static_cast<double>(kStartupTimeoutMs) / 1000.0);
}

void VideoController::checkWatchdog()
{
    if (!watchdog_.isArmed())
        return;

    const int64_t now = FrameClock::nowNs();

    if (!watchdog_.hasTimedOut(now))
        return;

    const int64_t frameAgeNs = watchdog_.frameAgeNs(now);

    stop();

    emit streamingLost(streamingInfo_, static_cast<double>(frameAgeNs) / 1e9);
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
    if (state_ == StreamingState::Stopped || startupInfo_.type != SourceType::Camera)
        return;

    const bool cameraStillExists = std::any_of(devices.begin(), devices.end(),
                                               [&](const QCameraDevice& dev)
                                               {
                                                   return dev.id() == startupInfo_.camera.deviceId;
                                               });

    if (!cameraStillExists)
        stop();
}

void VideoController::onCameraError(QCamera::Error error, const QString& errorString)
{
    const StreamingState state = state_;

    CameraErrorInfo camError;
    camError.error = error;
    camError.errorString = errorString;
    camError.sourceInfo = startupInfo_;
    camError.state = state;

    emit cameraError(camError);

    if (state == StreamingState::Starting)
    {
        // stop() disconnects camera-related callbacks. Defer it to the next event
        // loop iteration to avoid tearing down the camera pipeline while the
        // current error callback is still executing.
        QTimer::singleShot(0, this, &VideoController::stop);
    }
}

void VideoController::onMediaPlayerError(QMediaPlayer::Error error, const QString& errorString)
{
    const StreamingState state = state_;

    if (state == StreamingState::Starting)
        stop();

    MediaPlayerErrorInfo mediaError;
    mediaError.error = error;
    mediaError.errorString = errorString;
    mediaError.sourceInfo = startupInfo_;
    mediaError.state = state;

    emit mediaPlayerError(mediaError);
}

void VideoController::onVideoSettingsChanged(const VideoSessionSettings& session)
{
    downscaleParams_ = session.compute.downscale;

    processingThread_.setAlgoConfig(session.compute);

    emit downscaleChanged(session.compute.downscale);
}

void VideoController::onVideoDisplaySettingsChanged(const DisplayConfig& display)
{
    displayConfig_ = display;

    processingThread_.setDisplayMode(display.displayMode);
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

bool VideoController::isMediaActive() const
{
    return activeSource().type == SourceType::Media;
}

QList<QCameraDevice> VideoController::videoInputs() const
{
    return QMediaDevices::videoInputs();
}

qint64 VideoController::positionMs() const
{
    if (!isMediaActive())
        return 0;

    return mediaSource_.positionMs();
}

qint64 VideoController::durationMs() const
{
    if (!isMediaActive())
        return 0;

    return mediaSource_.durationMs();
}

void VideoController::seek(qint64 positionMs)
{
    if (!isMediaActive())
        return;

    positionMs = std::clamp(positionMs, 0LL, durationMs());

    watchdog_.reset();

    mediaSource_.setPosition(positionMs);
}

float VideoController::volume() const
{
    return audioOutput_.volume();
}

void VideoController::setVolume(float volume)
{
    volume = std::clamp(volume, 0.0f, 1.0f);

    if (qFuzzyCompare(audioOutput_.volume(), volume))
        return;

    audioOutput_.setMuted(false);
    audioOutput_.setVolume(volume);
}

bool VideoController::isMuted() const
{
    return audioOutput_.isMuted();
}

void VideoController::setMuted(bool muted)
{
    if (audioOutput_.isMuted() == muted)
        return;

    audioOutput_.setMuted(muted);
}

bool VideoController::isPaused() const
{
    return isMediaActive() && mediaSource_.isPaused();
}

void VideoController::pause()
{
    if (!isMediaActive())
        return;

    watchdog_.reset();

    mediaSource_.pause();
}

void VideoController::resume()
{
    if (!isMediaActive())
        return;

    mediaSource_.resume();
}

} // namespace fluvel
