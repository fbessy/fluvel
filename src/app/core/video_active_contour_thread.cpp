// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_active_contour_thread.hpp"

#include "frame_clock.hpp"
#include "image_adapters.hpp"

namespace fluvel_app
{

VideoActiveContourThread::VideoActiveContourThread(QObject* parent)
    : QThread(parent)

{
}

void VideoActiveContourThread::submitFrame(const QVideoFrame& frame)
{
    FrameData fd;
    fd.frame = frame;
    fd.receiveTs = FrameClock::nowNs();

    {
        QMutexLocker locker(&frameMutex_);
        lastFrameData_ = fd;
        frameAvailable_ = true;
    }

    condition_.wakeOne();
}

void VideoActiveContourThread::run()
{
    QMutexLocker locker(&frameMutex_);
    running_ = true;

    while (running_)
    {
        while (!frameAvailable_ && running_)
            condition_.wait(&frameMutex_);

        if (!running_)
            break;

        FrameData fd = lastFrameData_;
        frameAvailable_ = false;
        locker.unlock();

        FrameResult result = processFrame(fd.frame);

        result.receiveTs = fd.receiveTs;

        quint64 contourSize = static_cast<quint64>(result.l_out.size() + result.l_in.size());

        emit frameProcessed(contourSize);

        emit frameResultReady(result);

        locker.relock();
    }
}

FrameResult VideoActiveContourThread::processFrame(QVideoFrame& frame)
{
    qint64 startTs = FrameClock::nowNs();

    FrameResult fr;
    fr.input = frame.toImage();

    switch (fr.input.format())
    {
        case QImage::Format_RGB32:
        case QImage::Format_RGB888:
            break;

        default:
            fr.input = fr.input.convertToFormat(QImage::Format_RGB32);
            break;
    }

    if (!fr.input.isNull())
    {
        const auto& config = config_;

        // downscale
        const bool hasDownscale = config.downscale.hasDownscale;
        const int downscale_fctr = config.downscale.downscaleFactor;

        if (hasDownscale)
        {
            assert(downscale_fctr == 2 || downscale_fctr == 4);

            fr.preprocessed = fr.input.scaled(fr.input.width() / downscale_fctr,
                                              fr.input.height() / downscale_fctr,
                                              Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        else
        {
            fr.preprocessed = fr.input;
        }

        auto img_algo = image_span_from_qimage(fr.preprocessed);

        const int newW = img_algo.width();
        const int newH = img_algo.height();

        const bool sizeChanged = (newW != currentWidth_) || (newH != currentHeight_);

        if (!region_ac_ || configChanged_ || sizeChanged)
        {
            if (config.hasTemporalFiltering)
            {
                smoother_.reset(img_algo);
                img_algo = smoother_.outputSpan();
            }

            const auto& algo_conf = config.algo;

            region_ac_ = std::make_unique<fluvel_ip::RegionColorAc>(
                img_algo,
                fluvel_ip::ContourData(img_algo.width(), img_algo.height(), algo_conf.connectivity),
                algo_conf.acConfig, algo_conf.regionAcConfig);

            currentWidth_ = newW;
            currentHeight_ = newH;
            configChanged_ = false;

            QString size_str = QString("%1×%2").arg(QString::number(fr.preprocessed.width()),
                                                    QString::number(fr.preprocessed.height()));

            if (downscale_fctr >= 2)
            {
                size_str += QString(" /%1").arg(downscale_fctr);
            }

            emit frameSizeStr(size_str);
        }
        else
        {
            if (config.hasTemporalFiltering)
            {
                smoother_.update(img_algo);
                img_algo = smoother_.outputSpan();
            }

            region_ac_->resetExecutionState(img_algo);
        }

        region_ac_->runCycles(config.cyclesNbr);
        fr.processTs = FrameClock::nowNs() - startTs;

        if (region_ac_)
        {
            if (config.hasTemporalFiltering)
            {
                fr.preprocessed =
                    QImage(img_algo.data(), img_algo.width(), img_algo.height(),
                           static_cast<qsizetype>(3 * img_algo.width()), QImage::Format_RGB888);
            }

            fr.l_out = region_ac_->export_l_out();
            fr.l_in = region_ac_->export_l_in();
        }
    }

    return fr;
}

void VideoActiveContourThread::stop()
{
    QMutexLocker locker(&frameMutex_);
    running_ = false;
    condition_.wakeAll();
}

void VideoActiveContourThread::setAlgoConfig(const VideoComputeConfig& config)
{
    QMutexLocker locker(&frameMutex_);
    config_ = config;
    configChanged_ = true;
}

} // namespace fluvel_app
