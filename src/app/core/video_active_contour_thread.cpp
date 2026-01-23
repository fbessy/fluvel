#include "video_active_contour_thread.hpp"
#include "image_adapters.hpp"
#include "frame_clock.hpp"
#include "application_settings.hpp"
#include "contour_rendering_qimage.hpp"
#include "active_contour.hpp"

namespace ofeli_app {

VideoActiveContourThread::VideoActiveContourThread(QObject* parent)
    : QThread(parent),
    runtime_settings(AppSettings::instance().snapshot()),
    frameAvailable(false),
    running(true),
    configChanged(false)
{
    QObject::connect(
        &AppSettings::instance(),
        &ApplicationSettings::settingsApplied,
        this,
        &VideoActiveContourThread::reloadSettings,
        Qt::QueuedConnection
        );
}

void VideoActiveContourThread::submitFrame(const QVideoFrame& frame)
{
    FrameData fd;
    fd.frame = frame;
    fd.receiveTs = FrameClock::nowNs();

    {
        QMutexLocker locker(&frameMutex);
        lastFrameData = fd;
        frameAvailable = true;
    }

    condition.wakeOne();
}

void VideoActiveContourThread::run()
{
    QMutexLocker locker(&frameMutex);
    running = true;

    while (running)
    {
        while (!frameAvailable && running)
            condition.wait(&frameMutex);

        if (!running)
            break;

        FrameData fd = lastFrameData;
        frameAvailable = false;
        locker.unlock();

        qint64 processEndTs;
        QImage result = processFrame(fd.frame, processEndTs);

        emit frameProcessed(fd.receiveTs, processEndTs);
        emit frameResultReady(result, fd.receiveTs);

        locker.relock();
    }
}

QImage VideoActiveContourThread::processFrame(QVideoFrame& frame, qint64& processTs)
{
    qint64 startTs = FrameClock::nowNs();

    QImage result = frame.toImage().convertToFormat(QImage::Format_RGB32);

    if ( !result.isNull() )
    {
        const auto& config = runtime_settings;

        if (config.is_show_mirrored)
            result = result.flipped(Qt::Horizontal);

        // downscale
        QImage q_img_algo = result;
        unsigned int downscale_fctr = config.downscale_factor;
        if (downscale_fctr >= 2)
            q_img_algo = result.scaled(result.width()/downscale_fctr,
                                       result.height()/downscale_fctr,
                                       Qt::IgnoreAspectRatio,
                                       Qt::FastTransformation);

        const auto img_algo = image_span_from_qimage( q_img_algo );

        if ( !region_ac || configChanged )
        {
            ofeli_ip::AcConfig conf = config.algo_config;

            // a specific configuration for video tracking
            conf.failure_mode = ofeli_ip::FailureHandlingMode::RecoverOnFailure;

            region_ac = std::make_unique<ofeli_ip::RegionColorAc>(img_algo,
                                                                  ofeli_ip::ContourData(img_algo.width(),
                                                                                        img_algo.height(),
                                                                                        config.connectivity),
                                                                  conf,
                                                                  config.region_ac_config);

            configChanged = false;
        }
        else
        {
            region_ac->resetExecutionState(img_algo);
        }

        region_ac->run_cycles(config.cycles_nbr);

        if (region_ac)
        {
            if (downscale_fctr == 1)
            {
                draw_list_to_img(region_ac->l_out(), config.color_out, config.outside_combo,
                                 result);

                draw_list_to_img(region_ac->l_in(), config.color_in, config.inside_combo,
                                 result);
            }
            else
            {
                draw_upscale_list(region_ac->l_out(), config.color_out, config.outside_combo, downscale_fctr, result);
                draw_upscale_list(region_ac->l_in(), config.color_in, config.inside_combo, downscale_fctr, result);
            }
        }
    }

    processTs = FrameClock::nowNs() - startTs;
    return result;
}

void VideoActiveContourThread::stop()
{
    QMutexLocker locker(&frameMutex);
    running = false;
    condition.wakeAll();
}

void VideoActiveContourThread::reloadSettings()
{
    runtime_settings = AppSettings::instance().snapshot();
    configChanged = true;
}

} // namespace ofeli_app

