#include "video_active_contour_thread.hpp"
#include "image_adapters.hpp"
#include "frame_clock.hpp"
#include "application_settings.hpp"
#include "contour_rendering_qimage.hpp"
#include "active_contour.hpp"

namespace ofeli_app {

VideoActiveContourThread::VideoActiveContourThread(QObject* parent)
    : QThread(parent),
    frameAvailable(false),
    running(true),
    configChanged(false)
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

        FrameResult result = processFrame(fd.frame);

        result.receiveTs = fd.receiveTs;

        emit frameProcessed(fd.receiveTs,
                            result.processTs);

        emit frameResultReady(result);

        locker.relock();
    }
}

FrameResult VideoActiveContourThread::processFrame(QVideoFrame& frame)
{
    qint64 startTs = FrameClock::nowNs();

    FrameResult fr;
    fr.input = frame.toImage();

    switch ( fr.input.format() )
    {
    case QImage::Format_RGB32:
    case QImage::Format_RGB888:
        break;

    default:
        fr.input = fr.input.convertToFormat(QImage::Format_RGB32);
        break;
    }

    if ( !fr.input.isNull() )
    {
        const auto& config = config_;

        // downscale
        const bool hasDownscale = config.downscale.hasDownscale;
        const int downscale_fctr = config.downscale.downscaleFactor;

        if ( hasDownscale )
        {
            assert( downscale_fctr == 2 || downscale_fctr == 4 );

            fr.preprocessed = fr.input.scaled(fr.input.width() / downscale_fctr,
                                              fr.input.height() / downscale_fctr,
                                              Qt::IgnoreAspectRatio,
                                              Qt::FastTransformation);
        }
        else
        {
            fr.preprocessed = fr.input;
        }

        auto img_algo = image_span_from_qimage( fr.preprocessed );

        const int newW = img_algo.width();
        const int newH = img_algo.height();

        const bool sizeChanged =
            (newW != currentWidth_) ||
            (newH != currentHeight_);

        if ( !region_ac || configChanged || sizeChanged )
        {
            if ( config.hasTemporalFiltering )
            {
                smoother.reset( img_algo );
                img_algo = smoother.outputSpan();
            }

            const auto& algo_conf = config.algo;

            region_ac = std::make_unique<ofeli_ip::RegionColorAc>(img_algo,
                                                                  ofeli_ip::ContourData(img_algo.width(),
                                                                                        img_algo.height(),
                                                                                        algo_conf.connectivity),
                                                                  algo_conf.acConfig,
                                                                  algo_conf.regionAcConfig);

            currentWidth_  = newW;
            currentHeight_ = newH;
            configChanged = false;

            QString size_str = QString("%1×%2")
                                   .arg(QString::number(fr.preprocessed.width()),
                                        QString::number(fr.preprocessed.height()));

            if ( downscale_fctr >= 2 )
            {
                size_str += QString(" /%1").arg(downscale_fctr);
            }

            emit frameSizeStr(size_str);
        }
        else
        {
            if ( config.hasTemporalFiltering )
            {
                smoother.update( img_algo );
                img_algo = smoother.outputSpan();
            }

            region_ac->resetExecutionState( img_algo );
        }

        region_ac->run_cycles(config.cyclesNbr);
        fr.processTs = FrameClock::nowNs() - startTs;

        if ( region_ac )
        {
            if ( config.hasTemporalFiltering )
            {
                fr.preprocessed = QImage(img_algo.data(),
                                         img_algo.width(),
                                         img_algo.height(),
                                         3 * img_algo.width(),
                                         QImage::Format_RGB888);
            }

            fr.l_out = region_ac->export_l_out();
            fr.l_in  = region_ac->export_l_in();
        }
    }

    return fr;
}

void VideoActiveContourThread::stop()
{
    QMutexLocker locker(&frameMutex);
    running = false;
    condition.wakeAll();
}

void VideoActiveContourThread::setAlgoConfig(const VideoComputeConfig& config)
{
    QMutexLocker locker(&frameMutex);
    config_ = config;
    configChanged = true;
}

} // namespace ofeli_app

