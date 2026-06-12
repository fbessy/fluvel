// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_controller.hpp"
#include "camera_format_utils.hpp"
#include "contour_adapters.hpp"
#include "frame_clock.hpp"
#include "streaming_stats.hpp"

#include <QAudioOutput>
#include <QCamera>
#include <QDebug>
#include <QFileInfo>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMediaPlayer>
#include <QTimer>
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

    startupTimer_ = new QTimer(this);
    startupTimer_->setSingleShot(true);

    watchdogTimer_ = new QTimer(this);
    watchdogTimer_->setInterval(kWatchdogPeriodMs);

    diagnosticsTimer_ = new QTimer(this);
    diagnosticsTimer_->setInterval(kDiagnosticsPeriodMs);

    connect(startupTimer_, &QTimer::timeout, this, &VideoController::onStartupTimeout);

    connect(watchdogTimer_, &QTimer::timeout, this, &VideoController::checkWatchdog);

    connect(diagnosticsTimer_, &QTimer::timeout, this, &VideoController::updateDiagnostics);

    connect(&activeContourThread_, &VideoActiveContourThread::frameProcessed, this,
            &VideoController::onFrameProcessed);
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
        case SourceType::Url:
        case SourceType::File:
            start(sourceConfig.url);
            return;

        case SourceType::Camera:
            start(sourceConfig.cameraId, sourceConfig.cameraFormat);
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
    assert(startupTimer_);

    if (state_ != StreamingState::Stopped || camera_ || videoSink_ || captureSession_ ||
        mediaPlayer_)
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
            videoSink_ = new QVideoSink(this);
            captureSession_ = new QMediaCaptureSession(this);

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

            startupInfo_ = SourceInfo{};
            startupInfo_.type = SourceType::Camera;
            startupInfo_.deviceId = camera_->cameraDevice().id();
            startupInfo_.deviceFormat = camera_->cameraFormat();
            startupInfo_.description = camera_->cameraDevice().description();

            captureSession_->setCamera(camera_);
            captureSession_->setVideoSink(videoSink_);

            connect(camera_, &QCamera::errorOccurred, this, &VideoController::onCameraError);
            connect(videoSink_, &QVideoSink::videoFrameChanged, this,
                    &VideoController::onFrameReceived);

#ifdef FLUVEL_SIMULATE_STREAM_LOSS
            testFrameCounter_ = 0;
#endif

            startupTimer_->start(kStartupTimeoutMs);

            camera_->start();

            break;
        }
    }

    if (!isFound)
        emit cameraError(deviceId, QCamera::CameraError, tr("Camera not found"));
}

void VideoController::start(const QUrl& url)
{
    assert(startupTimer_);

    if (state_ != StreamingState::Stopped || camera_ || videoSink_ || captureSession_ ||
        mediaPlayer_)
        return;

    state_ = StreamingState::Starting;
    emit streamingStarting();

    startupInfo_ = SourceInfo{};
    startupInfo_.sourceUrl = url;

    if (startupInfo_.sourceUrl.isLocalFile())
        startupInfo_.type = SourceType::File;
    else
        startupInfo_.type = SourceType::Url;

    startupInfo_.description = shortSourceName(startupInfo_.sourceUrl);

    mediaPlayer_ = new QMediaPlayer(this);
    videoSink_ = new QVideoSink(this);

    // HTTP MP4 test source.
    //
    // Note:
    // This file can be downloaded and played locally,
    // but some media players (Qt Multimedia, VLC, mpv)
    // may fail to stream it directly because the MP4
    // is not suitable for seekable HTTP streaming
    // ("no moov before mdat and stream is not seekable").
    //
    // Useful regression test for URL media error handling.

    // mediaPlayer_->setSource(QUrl("/home/fabien/sample-20s.mp4"));

    // mediaPlayer_->setSource(QUrl("https://samplelib.com/preview/mp4/sample-20s.mp4"));

    /*mediaPlayer_->setSource(QUrl("https://test-videos.co.uk/vids/bigbuckbunny/mp4/h264/720/"
                                 "Big_Buck_Bunny_720_10s_1MB.mp4"));*/

    // mediaPlayer_->setSource(QUrl("https://test-streams.mux.dev/x36xhzz/x36xhzz.m3u8"));

    // mediaPlayer_->setSource(QUrl("http://192.168.1.110:8080/video"));

    // mediaPlayer_->setSource(QUrl("rtsp://localhost:8554/test"));

    // mediaPlayer_->setSource(QUrl("http://localhost:8888/test/index.m3u8"));

    mediaPlayer_->setSource(url);
    mediaPlayer_->setVideoSink(videoSink_);
    connect(videoSink_, &QVideoSink::videoFrameChanged, this, &VideoController::onFrameReceived);

#ifdef FLUVEL_SIMULATE_STREAM_LOSS
    testFrameCounter_ = 0;
#endif

    audioOutput_ = new QAudioOutput(this);
    audioOutput_->setVolume(0.5f);
    mediaPlayer_->setAudioOutput(audioOutput_);

    connect(mediaPlayer_, &QMediaPlayer::mediaStatusChanged, this,
            &VideoController::onMediaStatusChanged);

    startupTimer_->start(kStartupTimeoutMs);

    mediaPlayer_->play();
}

void VideoController::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia)
    {
        stop();
    }
}

void VideoController::stop()
{
    assert(startupTimer_ && watchdogTimer_ && diagnosticsTimer_);

    if (state_ == StreamingState::Stopped)
        return;

    startupTimer_->stop();
    watchdogTimer_->stop();
    diagnosticsTimer_->stop();

    if (videoSink_)
    {
        disconnect(videoSink_, &QVideoSink::videoFrameChanged, this,
                   &VideoController::onFrameReceived);
    }

    if (camera_)
    {
        disconnect(camera_, &QCamera::errorOccurred, this, &VideoController::onCameraError);
        camera_->stop();
    }

    if (captureSession_)
    {
        captureSession_->setVideoSink(nullptr);
        captureSession_->setCamera(nullptr);

        delete captureSession_;
        captureSession_ = nullptr;
    }

    if (mediaPlayer_)
    {
        mediaPlayer_->stop();

        delete mediaPlayer_;

        mediaPlayer_ = nullptr;
    }

    if (videoSink_)
    {
        delete videoSink_;
        videoSink_ = nullptr;
    }

    if (camera_)
    {
        delete camera_;
        camera_ = nullptr;
    }

    state_ = StreamingState::Stopped;
    emit streamingStopped();
}

void VideoController::onFrameReceived(const QVideoFrame& frame)
{
    assert(startupTimer_ && watchdogTimer_ && diagnosticsTimer_);

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

    if (state_ == StreamingState::Starting)
    {
        state_ = StreamingState::Streaming;

        startupTimer_->stop();
        watchdogTimer_->start();

        frameStats_.reset();
        diagnosticsTimer_->start();

        streamingInfo_ = StreamingInfo{};
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

    const bool cameraStillExists =
        std::any_of(devices.begin(), devices.end(),
                    [&](const QCameraDevice& dev)
                    {
                        return dev.id() == streamingInfo_.source.deviceId;
                    });

    if (!cameraStillExists)
        stop();
}

void VideoController::onCameraError(QCamera::Error error, const QString& errorString)
{
    if (!camera_)
        return;

    emit cameraError(camera_->cameraDevice().id(), error, errorString);
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

QList<QCameraDevice> VideoController::videoInputs() const
{
    return QMediaDevices::videoInputs();
}

QString VideoController::shortSourceName(const QUrl& url)
{
    if (url.isLocalFile())
        return QFileInfo(url.toLocalFile()).fileName();

    return url.fileName();
}

} // namespace fluvel
