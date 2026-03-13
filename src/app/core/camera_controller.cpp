// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "camera_controller.hpp"
#include "camera_stats.hpp"
#include "contour_adapters.hpp"
#include "frame_clock.hpp"

#include <QCamera>
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

    connect(&activeContourThread_, &VideoActiveContourThread::frameResultReady, this,
            &CameraController::onFrameResultReady, Qt::QueuedConnection);

    connect(&activeContourThread_, &VideoActiveContourThread::frameSizeStr, this,
            &CameraController::frameSizeStr);

    activeContourThread_.start();

    statsTimer_ = new QTimer(this);
    statsTimer_->setInterval(500);

    connect(statsTimer_, &QTimer::timeout, this, &CameraController::updateDiagnostics);

    connect(&activeContourThread_, &VideoActiveContourThread::frameProcessed, this,
            &CameraController::onFrameProcessed);
}

CameraController::~CameraController()
{
    stop();
    activeContourThread_.stop();
    activeContourThread_.wait();
}

bool CameraController::isActive() const
{
    return streamStarted_;
}

void CameraController::start(const QByteArray& deviceId)
{
    stop();

    const auto cameras = QMediaDevices::videoInputs();
    bool isFound = false;

    for (const auto& cam : cameras)
    {
        if (cam.id() == deviceId)
        {
            isFound = true;

            camera_ = new QCamera(cam, this);
            activeDeviceId_ = cam.id();

            videoSink_ = new QVideoSink(this);
            captureSession_ = new QMediaCaptureSession(this);

            captureSession_->setCamera(camera_);
            captureSession_->setVideoSink(videoSink_);

            connect(camera_, &QCamera::activeChanged, this,
                    &CameraController::onCameraActiveChanged);

            connect(camera_, &QCamera::errorOccurred, this, &CameraController::onCameraError);

            connect(videoSink_, &QVideoSink::videoFrameChanged, this,
                    &CameraController::onVideoFrame);

            camera_->start();

            break;
        }
    }

    if (!isFound)
        emit cameraError(deviceId, QCamera::CameraError, tr("Camera not found"));
}

void CameraController::stop()
{
    streamStarted_ = false;
    activeDeviceId_.clear();
    statsTimer_->stop();

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
}

void CameraController::onFrameProcessed(quint64 contourSize)
{
    frameStats_.frameProcessed(contourSize);
}

void CameraController::onFrameDisplayed(qint64 recvTsNs, qint64 displayTsNs)
{
    frameStats_.frameDisplayed(recvTsNs, displayTsNs);
}

void CameraController::onFrameResultReady(const FrameResult& result)
{
    auto q_l_out = convertToQVector(result.l_out);
    auto q_l_in = convertToQVector(result.l_in);

    QImage img;

    if (displayConfig_.image == ImageBase::Source)
        img = result.input;
    else if (displayConfig_.image == ImageBase::Preprocessed)
        img = result.preprocessed;

    if (!img.isNull())
        emit imageAndContourUpdated(img, q_l_out, q_l_in, result.receiveTs);
}

void CameraController::updateDiagnostics()
{
    auto snap = frameStats_.snapshot();

    CameraStats stats{snap.inputFps,     snap.processingFps, snap.displayFps,    snap.dropRate,
                      snap.avgLatencyMs, snap.maxLatencyMs,  snap.avgContourSize};

    QString textStats = QString(tr("In | Proc | Disp: %1 | %2 | %3 fps\n"
                                   "Lat: %4 ms (max %5) | Drop: %6 %\n"
                                   "Contour: %7 pts"))
                            .arg(stats.inputFps, 0, 'f', 1)
                            .arg(stats.processingFps, 0, 'f', 1)
                            .arg(stats.displayFps, 0, 'f', 1)
                            .arg(stats.avgLatencyMs, 0, 'f', 1)
                            .arg(stats.maxLatencyMs, 0, 'f', 1)
                            .arg(100.f * stats.dropRate, 0, 'f', 1)
                            .arg(stats.avgContourSize, 0, 'f', 0);

    emit textStatsUpdated(textStats);
}

void CameraController::onVideoSettingsChanged(const VideoSessionSettings& session)
{
    activeContourThread_.setAlgoConfig(session.compute);
}

void CameraController::onVideoDisplaySettingsChanged(const DisplayConfig& display)
{
    displayConfig_ = display;
}

void CameraController::onCameraActiveChanged(bool active)
{
    if (!camera_)
        return;

    if (!active)
        emit cameraStopped(camera_->cameraDevice().id());
}

void CameraController::onVideoInputsChanged()
{
    if (activeDeviceId_.isEmpty())
        return;

    auto inputs = QMediaDevices::videoInputs();

    bool cameraStillExists = std::any_of(inputs.begin(), inputs.end(),
                                         [&](const QCameraDevice& dev)
                                         {
                                             return dev.id() == activeDeviceId_;
                                         });

    if (!cameraStillExists)
    {
        stop();
    }
}

void CameraController::onCameraError(QCamera::Error error, const QString& errorString)
{
    if (!camera_)
        return;

    emit cameraError(camera_->cameraDevice().id(), error, errorString);
}

void CameraController::onVideoFrame(const QVideoFrame& frame)
{
    if (!frame.isValid())
        return;

    if (!streamStarted_)
    {
        streamStarted_ = true;
        frameStats_.reset();
        statsTimer_->start();

        emit cameraStarted(activeDeviceId_);
    }

    const qint64 recvTs = FrameClock::nowNs();
    frameStats_.frameReceived(recvTs);
    activeContourThread_.submitFrame(frame);
}

} // namespace fluvel_app
