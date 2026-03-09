// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "camera_controller.hpp"

#include "application_settings.hpp"
#include "camera_stats.hpp"
#include "contour_adapters.hpp"
#include "frame_clock.hpp"

namespace fluvel_app
{

CameraController::CameraController(QObject* parent)
    : QObject(parent)
    , ac_thread_(this)
{
    onVideoSettingsChanged(ApplicationSettings::instance().videoSettings());
    onVideoDisplaySettingsChanged(ApplicationSettings::instance().videoSettings().display);

    connect(&ApplicationSettings::instance(), &ApplicationSettings::videoSettingsChanged, this,
            &CameraController::onVideoSettingsChanged);

    connect(&ApplicationSettings::instance(), &ApplicationSettings::videoDisplaySettingsChanged,
            this, &CameraController::onVideoDisplaySettingsChanged);

    connect(&ac_thread_, &VideoActiveContourThread::frameResultReady, this,
            &CameraController::onFrameResultReady, Qt::QueuedConnection);

    connect(&ac_thread_, &VideoActiveContourThread::frameSizeStr, this,
            &CameraController::frameSizeStr);

    ac_thread_.start();

    // Stats timer
    statsTimer_ = new QTimer(this);
    statsTimer_->setInterval(500);

    connect(statsTimer_, &QTimer::timeout, this, &CameraController::updateDiagnostics);

    // Thread → controller
    connect(&ac_thread_, &VideoActiveContourThread::frameProcessed, this,
            &CameraController::onFrameProcessed);
}

CameraController::~CameraController()
{
    stop();
    ac_thread_.stop();
    ac_thread_.wait();
}

bool CameraController::isActive() const
{
    return camera_ && camera_->isActive();
}

void CameraController::start(const QByteArray& deviceId)
{
    stop();

    const auto cameras = QMediaDevices::videoInputs();

    for (const auto& cam : cameras)
    {
        if (cam.id() == deviceId)
        {
            camera_ = new QCamera(cam, this);
            videoSink_ = new QVideoSink(this);
            captureSession_ = new QMediaCaptureSession(this);

            captureSession_->setCamera(camera_);
            captureSession_->setVideoSink(videoSink_);

            connect(videoSink_, &QVideoSink::videoFrameChanged, this,
                    [this](const QVideoFrame& frame)
                    {
                        const qint64 recvTs = FrameClock::nowNs();
                        frameStats_.frameReceived(recvTs);
                        ac_thread_.submitFrame(frame);
                    });

            camera_->start();

            frameStats_.reset();
            statsTimer_->start();

            return;
        }
    }
}

void CameraController::stop()
{
    if (statsTimer_)
        statsTimer_->stop();

    if (camera_)
    {
        camera_->stop();
        camera_->deleteLater();
        camera_ = nullptr;
    }

    if (videoSink_)
    {
        videoSink_->deleteLater();
        videoSink_ = nullptr;
    }

    if (captureSession_)
    {
        captureSession_->deleteLater();
        captureSession_ = nullptr;
    }
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

void CameraController::onVideoSettingsChanged(const VideoSessionSettings& conf)
{
    ac_thread_.setAlgoConfig(conf.compute);
}

void CameraController::onVideoDisplaySettingsChanged(const DisplayConfig& displayConfig)
{
    displayConfig_ = displayConfig;
}

} // namespace fluvel_app
