// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "camera_controller.hpp"
#include "camera_stats.hpp"
#include "contour_adapters.hpp"
#include "frame_clock.hpp"

#include <QCamera>
#include <QDebug>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QTimer>
#include <QVideoSink>

namespace fluvel_app
{

CameraController::CameraController(const VideoSessionSettings& session, QObject* parent)
    : QObject(parent)
    , activeContourThread_(this)
{
    auto mediaDevices = new QMediaDevices(this);

    connect(mediaDevices, &QMediaDevices::videoInputsChanged, this,
            &CameraController::onVideoInputsChanged);

    onVideoSettingsChanged(session);
    onVideoDisplaySettingsChanged(session.display);

    connect(&activeContourThread_, &VideoActiveContourThread::displayFrameReady, this,
            &CameraController::onDisplayFrameReady, Qt::QueuedConnection);

    activeContourThread_.start();

    startupTimer_ = new QTimer(this);
    startupTimer_->setSingleShot(true);

    watchdogTimer_ = new QTimer(this);
    watchdogTimer_->setInterval(kWatchdogPeriodMs);

    diagnosticsTimer_ = new QTimer(this);
    diagnosticsTimer_->setInterval(kDiagnosticsPeriodMs);

    connect(startupTimer_, &QTimer::timeout, this, &CameraController::onStartupTimeout);

    connect(watchdogTimer_, &QTimer::timeout, this, &CameraController::checkWatchdog);

    connect(diagnosticsTimer_, &QTimer::timeout, this, &CameraController::updateDiagnostics);

    connect(&activeContourThread_, &VideoActiveContourThread::frameProcessed, this,
            &CameraController::onFrameProcessed);
}

CameraController::~CameraController()
{
    stop();
    activeContourThread_.stop();
    activeContourThread_.wait();
}

void CameraController::start(const QByteArray& deviceId, const QCameraFormat& format)
{
    if (state_ != StreamingState::Stopped)
        stop();

    state_ = StreamingState::Starting;

    const auto cameras = QMediaDevices::videoInputs();
    bool isFound = false;

    for (const auto& cam : cameras)
    {
#ifdef FLUVEL_DEBUG
        for (const QCameraFormat& f : cam.videoFormats())
        {
            qDebug() << "format:" << QVideoFrameFormat::pixelFormatToString(f.pixelFormat())
                     << "resolution:" << f.resolution() << "fps:" << f.maxFrameRate();
        }
#endif

        if (cam.id() == deviceId)
        {
            isFound = true;

            camera_ = new QCamera(cam, this);
            deviceId_ = cam.id();

            videoSink_ = new QVideoSink(this);
            captureSession_ = new QMediaCaptureSession(this);

            // 👉 Application du format choisi (UI)
            if (!format.isNull())
            {
                const auto formats = cam.videoFormats();

                auto it = std::find_if(formats.begin(), formats.end(),
                                       [&](const QCameraFormat& f)
                                       {
                                           return f.pixelFormat() == format.pixelFormat() &&
                                                  f.resolution() == format.resolution() &&
                                                  f.maxFrameRate() == format.maxFrameRate();
                                       });

                if (it != formats.end())
                {
                    camera_->setCameraFormat(*it);

#ifdef FLUVEL_DEBUG
                    qDebug() << "Selected UI format:"
                             << QVideoFrameFormat::pixelFormatToString(it->pixelFormat())
                             << it->resolution() << it->maxFrameRate();
#endif
                }
                else
                {
#ifdef FLUVEL_DEBUG
                    qDebug() << "UI format not found on device, using default";
#endif
                }
            }

            captureSession_->setCamera(camera_);
            captureSession_->setVideoSink(videoSink_);

            connect(camera_, &QCamera::errorOccurred, this, &CameraController::onCameraError);

            connect(videoSink_, &QVideoSink::videoFrameChanged, this,
                    &CameraController::onCapturedFrame);

#ifdef FLUVEL_SIMULATE_STREAM_LOSS
            testFrameCounter_ = 0;
#endif

            startupTimer_->start(kStartupTimeoutMs);
            camera_->start();

            break;
        }
    }

    if (!isFound)
    {
        emit cameraError(deviceId, QCamera::CameraError, tr("Camera not found"));
    }
}

QCameraFormat CameraController::chooseBestFormat(const QCameraDevice& dev)
{
    QCameraFormat bestYuv;
    QCameraFormat bestJpeg;

    for (const QCameraFormat& f : dev.videoFormats())
    {
        const int fps = static_cast<int>(f.maxFrameRate());
        const QSize res = f.resolution();
        const auto fmt = f.pixelFormat();

        // priorité : YUYV avec bon fps
        if (fmt == QVideoFrameFormat::Format_YUYV && fps >= 25)
        {
            if (bestYuv.isNull() || res.width() * res.height() > bestYuv.resolution().width() *
                                                                     bestYuv.resolution().height())
            {
                bestYuv = f;
            }
        }

        // fallback MJPEG
        if (fmt == QVideoFrameFormat::Format_Jpeg && fps >= 25)
        {
            if (bestJpeg.isNull() ||
                res.width() * res.height() >
                    bestJpeg.resolution().width() * bestJpeg.resolution().height())
            {
                bestJpeg = f;
            }
        }
    }

    if (!bestYuv.isNull())
        return bestYuv;

    if (!bestJpeg.isNull())
        return bestJpeg;

    return dev.videoFormats().isEmpty() ? QCameraFormat() : dev.videoFormats().first();
}

void CameraController::stop()
{
    if (state_ == StreamingState::Stopped)
        return;

    startupTimer_->stop();
    watchdogTimer_->stop();
    diagnosticsTimer_->stop();

    if (camera_)
        camera_->stop();

    if (captureSession_)
        captureSession_->deleteLater();

    if (videoSink_)
        videoSink_->deleteLater();

    if (camera_)
        camera_->deleteLater();

    camera_ = nullptr;
    videoSink_ = nullptr;
    captureSession_ = nullptr;

    state_ = StreamingState::Stopped;
    emit streamingStopped();
}

void CameraController::onCapturedFrame(const QVideoFrame& frame)
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

    const qint64 now = FrameClock::nowNs();
    lastValidFrameTsNs_ = now;

    if (state_ == StreamingState::Starting)
    {
        state_ = StreamingState::Streaming;

        startupTimer_->stop();
        watchdogTimer_->start();

        frameStats_.reset();
        diagnosticsTimer_->start();

        if (camera_)
        {
            StreamingInfo info;

            auto device = camera_->cameraDevice();
            info.deviceId = device.id();
            info.description = device.description();

            info.format = camera_->cameraFormat();

            emit streamingStarted(info);
        }
    }

    frameStats_.frameCaptured();

    CapturedFrame cf;
    cf.frame = frame;
    cf.receiveTimestampNs = now;

    activeContourThread_.submitFrame(cf);
}

void CameraController::onFrameProcessed(quint64 contourSize)
{
    frameStats_.frameProcessed(contourSize);
}

void CameraController::onFrameDisplayed(const FrameTimestamps& ts)
{
    frameStats_.frameDisplayed(ts);
}

void CameraController::onDisplayFrameReady(const DisplayFrame& displayFrame)
{
    UiFrame uiDisplayFrame;

    if (displayConfig_.image == ImageBase::Source)
        uiDisplayFrame.image = displayFrame.input;
    else if (displayConfig_.image == ImageBase::Preprocessed)
        uiDisplayFrame.image = displayFrame.preprocessed;

    if (uiDisplayFrame.image.isNull())
        return;

    uiDisplayFrame.outerContour = convertToQVector(displayFrame.outerContour);
    uiDisplayFrame.innerContour = convertToQVector(displayFrame.innerContour);

    uiDisplayFrame.receiveTimestampNs = displayFrame.receiveTimestampNs;
    uiDisplayFrame.processTimestampNs = displayFrame.processTimestampNs;

    emit imageAndContourUpdated(uiDisplayFrame);
}

void CameraController::onStartupTimeout()
{
    stop();
    emit startupTimeout(deviceId_, static_cast<double>(kStartupTimeoutMs) / 1000.0);
}

void CameraController::checkWatchdog()
{
    const qint64 frameAgeNs = FrameClock::nowNs() - lastValidFrameTsNs_;

    if (frameAgeNs > kStreamLossTimeoutNs)
    {
        stop();
        emit streamingLost(deviceId_, static_cast<double>(frameAgeNs) / 1e9);
    }
}

void CameraController::updateDiagnostics()
{
    auto snap = frameStats_.snapshot();

    CameraStats stats{snap.capturedFps,      snap.processedFps,        snap.displayedFps,
                      snap.dropRate,         snap.avgLatencyDisplayMs, snap.maxLatencyDisplayMs,
                      snap.avgLatencyProcMs, snap.avgContourSize};

    QString textStats = QString(tr("In | Proc | Disp: %1 | %2 | %3 fps\n"
                                   "Lat: %4 ms (proc %5) | Drop: %6 %\n"
                                   "Contour: %7 pts"))
                            .arg(stats.capturedFps, 0, 'f', 1)
                            .arg(stats.processedFps, 0, 'f', 1)
                            .arg(stats.displayedFps, 0, 'f', 1)
                            .arg(stats.avgLatencyDisplayMs, 0, 'f', 1)
                            .arg(stats.avgLatencyProcMs, 0, 'f', 1)
                            .arg(100.f * stats.dropRate, 0, 'f', 1)
                            .arg(stats.avgContourSize, 0, 'f', 0);

    emit textStatsUpdated(textStats);
}

void CameraController::onVideoInputsChanged()
{
    const auto inputs = QMediaDevices::videoInputs();

    handleActiveDeviceUnplug(inputs);

    emit videoInputsChanged(inputs);
}

void CameraController::handleActiveDeviceUnplug(const QList<QCameraDevice>& inputs)
{
    if (state_ == StreamingState::Stopped)
        return;

    const bool cameraStillExists = std::any_of(inputs.begin(), inputs.end(),
                                               [&](const QCameraDevice& dev)
                                               {
                                                   return dev.id() == deviceId_;
                                               });

    if (!cameraStillExists)
        stop();
}

void CameraController::onCameraError(QCamera::Error error, const QString& errorString)
{
    if (!camera_)
        return;

    emit cameraError(camera_->cameraDevice().id(), error, errorString);
}

void CameraController::onVideoSettingsChanged(const VideoSessionSettings& session)
{
    useOptimizedFormat_ = session.compute.useOptimizedFormat;
    activeContourThread_.setAlgoConfig(session.compute);
}

void CameraController::onVideoDisplaySettingsChanged(const DisplayConfig& display)
{
    displayConfig_ = display;
}

bool CameraController::isStreaming() const
{
    return state_ == StreamingState::Streaming;
}

QList<QCameraDevice> CameraController::videoInputs() const
{
    return QMediaDevices::videoInputs();
}

} // namespace fluvel_app
