#include "camera_controller.hpp"
#include "frame_clock.hpp"
#include "application_settings.hpp"
#include "contour_adapters.hpp"

namespace ofeli_app
{

CameraController::CameraController(QObject* parent)
    : QObject(parent),
      ac_thread_(this)
{
    onVideoSettingsChanged( AppSettings::instance().camConfig );
    onVideoDisplaySettingsChanged( AppSettings::instance().camConfig.display );

    connect(&AppSettings::instance(),
            &ApplicationSettings::videoSettingsChanged,
            this,
            &CameraController::onVideoSettingsChanged);

    connect(&AppSettings::instance(),
            &ApplicationSettings::videoDisplaySettingsChanged,
            this,
            &CameraController::onVideoDisplaySettingsChanged);

    connect(&ac_thread_,
            &VideoActiveContourThread::frameResultReady,
            this,
            &CameraController::onFrameResultReady,
            Qt::QueuedConnection);

    connect(&ac_thread_,
            &VideoActiveContourThread::frameSizeStr,
            this,
            &CameraController::frameSizeStr);



    ac_thread_.start();

    // Stats timer
    statsTimer_ = new QTimer(this);
    statsTimer_->setInterval(500);

    connect(statsTimer_,
            &QTimer::timeout,
            this,
            &CameraController::updateStats);

    // Thread → controller
    connect(&ac_thread_,
            &VideoActiveContourThread::frameProcessed,
            this,
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

void CameraController::onFrameDisplayed(qint64 recvTsNs,
                                        qint64 displayTsNs)
{
    frameStats_.frameDisplayed(recvTsNs, displayTsNs);
}

void CameraController::onFrameResultReady(FrameResult result)
{
    auto q_l_out = convertToQVector( result.l_out );
    auto q_l_in  = convertToQVector( result.l_in );

    QImage img;

    if ( displayConfig_.input_displayed )
        img = result.input;
    else
        img = result.preprocessed;

    if ( !img.isNull() )
        emit imageAndContourUpdated( img,
                                     q_l_out,
                                     q_l_in,
                                     result.receiveTs );
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

void CameraController::onVideoSettingsChanged(const VideoSessionSettings& conf)
{
    ac_thread_.setAlgoConfig( conf.compute );
}

void CameraController::onVideoDisplaySettingsChanged(const DisplayConfig& displayConfig)
{
    displayConfig_ = displayConfig;
}

} // namespace
