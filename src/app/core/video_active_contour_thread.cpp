#include "video_active_contour_thread.hpp"
#include "image_adapters.hpp"
#include "frame_clock.hpp"
#include "application_settings.hpp"
#include "contour_rendering_qimage.hpp"
#include "active_contour.hpp"

namespace ofeli_app {

VideoActiveContourThread::VideoActiveContourThread(QObject* parent)
    : QThread(parent),
    snapshot_settings(AppSettings::instance().camSessSettings),
    frameAvailable(false),
    running(true),
    configChanged(false)
{
    QObject::connect(
        &AppSettings::instance(),
        &ApplicationSettings::camSettingsApplied,
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

    QImage input = frame.toImage().convertToFormat(QImage::Format_RGB32);
    QImage result;

    if ( !input.isNull() )
    {
        const auto& config = snapshot_settings;

        if (config.has_show_mirrored)
            input = input.flipped(Qt::Horizontal);

        // downscale
        QImage q_img_algo = input;
        bool has_downscale = config.downscale_conf.has_downscale;
        int downscale_fctr = config.downscale_conf.downscale_factor;

        has_downscale = false;

        if ( has_downscale )
        {
            assert( downscale_fctr == 2 || downscale_fctr == 4 );

            q_img_algo = input.scaled(input.width() / downscale_fctr,
                                      input.height() / downscale_fctr,
                                      Qt::IgnoreAspectRatio,
                                      Qt::FastTransformation);
        }

        auto img_algo = image_span_from_qimage( q_img_algo );

        if ( !region_ac || configChanged )
        {
            if ( config.has_temporal_filtering )
                smoother.reset( img_algo );

            const auto& conf = config.cam_algo_conf;

            region_ac = std::make_unique<ofeli_ip::RegionColorAc>(img_algo,
                                                                  ofeli_ip::ContourData(img_algo.width(),
                                                                                        img_algo.height(),
                                                                                        conf.connectivity),
                                                                  conf.ac_config,
                                                                  conf.region_ac_config);

            configChanged = false;

            QString size_str = QString("%1×%2")
                                   .arg(QString::number(input.width()),
                                        QString::number(input.height()));

            if ( downscale_fctr >= 2 )
            {
                size_str += QString(" /%1").arg(downscale_fctr);
            }

            emit frameSizeStr(size_str);
        }
        else
        {
            if ( config.has_temporal_filtering )
            {
                smoother.update( img_algo );
                img_algo = smoother.outputSpan();
            }

            region_ac->resetExecutionState( img_algo );
        }

        region_ac->run_cycles(config.cycles_nbr);

        if (region_ac)
        {
            const auto& display_config = config.cam_disp_conf;

            result = input;

            if ( has_downscale )
            {
                if ( display_config.display_l_out )
                {
                    draw_upscale_list(region_ac->l_out(),
                                      display_config.l_out_color,
                                      downscale_fctr, result);
                }

                if ( display_config.display_l_in )
                {
                    draw_upscale_list(region_ac->l_in(),
                                      display_config.l_in_color,
                                      downscale_fctr, result);
                }
            }
            else
            {
                if ( display_config.display_l_out )
                {
                    draw_list_to_img(region_ac->l_out(),
                                     display_config.l_out_color,
                                     result);
                }

                if ( display_config.display_l_in )
                {
                    draw_list_to_img(region_ac->l_in(),
                                     display_config.l_in_color,
                                     result);
                }
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
    snapshot_settings = AppSettings::instance().camSessSettings;
    configChanged = true;
}

} // namespace ofeli_app

