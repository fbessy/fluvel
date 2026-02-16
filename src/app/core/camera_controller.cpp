#include "camera_controller.hpp"
#include "frame_clock.hpp"
#include "application_settings.hpp"

namespace ofeli_app
{

CameraController::CameraController(QObject* parent)
    : QObject(parent),
      ac_thread_(this)
{
    // Thread → controller
    connect(&ac_thread_,
            &VideoActiveContourThread::frameProcessed,
            this,
            &CameraController::onFrameProcessed,
            Qt::QueuedConnection);

    connect(&ac_thread_,
            &VideoActiveContourThread::frameResultReady,
            this,
            &CameraController::onFrameResultReady,
            Qt::QueuedConnection);

    connect(&ac_thread_,
            &VideoActiveContourThread::frameSizeStr,
            this,
            &CameraController::frameSizeStr);

    onCamSettingsChanged( AppSettings::instance().camSessSettings );
    onCamDisplaySettingsChanged( AppSettings::instance().camSessSettings.cam_disp_conf );

    connect(&AppSettings::instance(),
            &ApplicationSettings::camSettingsChanged,
            this,
            &CameraController::onCamSettingsChanged);

    connect(&AppSettings::instance(),
            &ApplicationSettings::camDisplaySettingsChanged,
            this,
            &CameraController::onCamDisplaySettingsChanged);

    ac_thread_.start();

    // Stats timer
    statsTimer_ = new QTimer(this);
    statsTimer_->setInterval(500);

    connect(statsTimer_,
            &QTimer::timeout,
            this,
            &CameraController::updateStats);
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

            connect(videoSink_,
                    &QVideoSink::videoFrameChanged,
                    this,
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

void CameraController::onFrameProcessed()
{
    frameStats_.frameProcessed();
}

void CameraController::onFrameResultReady(const QImage& img,
                                          qint64 recvTs)
{
    emit imageReady(img);

    const qint64 displayTs = FrameClock::nowNs();
    frameStats_.frameDisplayed(recvTs, displayTs);
}

void CameraController::updateStats()
{
    auto snap = frameStats_.snapshot();

    CameraStats stats {
        snap.inputFps,
        snap.processingFps,
        snap.displayFps,
        snap.dropRate,
        snap.avgLatencyMs,
        snap.maxLatencyMs
    };

    emit statsUpdated(stats);
}

void CameraController::onCamSettingsChanged(const CameraSessionSettings& conf)
{
    ac_thread_.setAlgoConfig( conf );
}

void CameraController::onCamDisplaySettingsChanged(const DisplayConfig& disp_config)
{
    ac_thread_.setDisplayConfig( disp_config );
}

} // namespace
