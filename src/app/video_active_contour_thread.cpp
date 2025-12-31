#include "video_active_contour_thread.hpp"
#include "image_adapters.hpp"
#include "frame_clock.hpp"

namespace ofeli_gui {

VideoActiveContourThread::VideoActiveContourThread(QObject* parent, const ApplicationSettings& config1)
    : QThread(parent), config(config1), frameAvailable(false), running(true)
{
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

    if (!result.isNull())
    {
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

        auto img_algo = image32_view_from_qimage(q_img_algo);

        if ( !region_ac || region_ac->get_phi().get_width() != img_algo.get_width()
             || region_ac->get_phi().get_height() != img_algo.get_height())
        {
            region_ac.reset(new ofeli_ip::RegionColorAc(img_algo,
                                                        ofeli_ip::ContourData(img_algo.get_width(), img_algo.get_height()),
                                                        config.algo_config,
                                                        config.region_ac_config));
        }
        else
        {
            region_ac->reinitialize(img_algo);
            region_ac->evolve_n_cycles(config.cycles_nbr);
        }

        if (region_ac)
        {
            if (downscale_fctr == 1)
            {
                draw_list_to_img(region_ac->get_l_out(), config.color_out, config.outside_combo,
                                 result.bits(), result.width(), result.height());
                draw_list_to_img(region_ac->get_l_in(), config.color_in, config.inside_combo,
                                 result.bits(), result.width(), result.height());
            }
            else
            {
                draw_upscale_list(region_ac->get_l_out(), config.color_out, config.outside_combo, downscale_fctr, result);
                draw_upscale_list(region_ac->get_l_in(), config.color_in, config.inside_combo, downscale_fctr, result);
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

} // namespace ofeli_gui

