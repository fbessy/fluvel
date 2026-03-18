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
    CapturedFrame fd;
    fd.frame = frame;
    fd.receiveTimestampNs = FrameClock::nowNs();

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

        CapturedFrame fd = lastFrameData_;
        frameAvailable_ = false;
        locker.unlock();

        DisplayFrame result = processFrame(fd.frame);

        result.receiveTimestampNs = fd.receiveTimestampNs;

        quint64 contourSize = static_cast<quint64>(result.outerContour.size() + result.innerContour.size());

        emit frameProcessed(contourSize);

        emit frameResultReady(result);

        locker.relock();
    }
}

QImage VideoActiveContourThread::convertFrame(QVideoFrame frame) const
{
    QImage img = frame.toImage();

    if (img.isNull())
        return img;

    switch (img.format())
    {
        case QImage::Format_RGB32:
        case QImage::Format_RGB888:
            return img;

        default:
            return img.convertToFormat(QImage::Format_RGB32);
    }
}

QImage VideoActiveContourThread::applyDownscale(const QImage& input,
                                                const DownscaleConfig& config) const
{
    if (input.isNull())
        return input;

    if (!config.hasDownscale)
        return input;

    const int factor = config.downscaleFactor;

    assert(factor == 2 || factor == 4);

    return input.scaled(input.width() / factor, input.height() / factor, Qt::IgnoreAspectRatio,
                        Qt::SmoothTransformation);
}

DisplayFrame VideoActiveContourThread::processFrame(const QVideoFrame& frame)
{
    qint64 startTs = FrameClock::nowNs();

    VideoComputeConfig config;

    {
        QMutexLocker locker(&frameMutex_);
        config = config_;
    }

    DisplayFrame fr;
    fr.input = convertFrame(frame);

    if (fr.input.isNull())
        return fr;

    fr.preprocessed = applyDownscale(fr.input, config.downscale);

    if (fr.preprocessed.isNull())
        return fr;

    auto algoImage = image_view_from_qimage(fr.preprocessed);
    const auto& algoConfig = config.algo;

    const auto newWSize = fr.preprocessed.size();

    if (!activeContour_ || configChanged_ || newWSize != currentSize_)
    {
        if (config.hasTemporalFiltering)
        {
            smoother_.reset(algoImage);
            algoImage = smoother_.outputSpan();
        }

        activeContour_ = std::make_unique<fluvel_ip::RegionColorAc>(
            algoImage,
            fluvel_ip::ContourData(algoImage.width(), algoImage.height(), algoConfig.connectivity),
            algoConfig.acConfig, algoConfig.regionAcConfig);

        currentSize_ = newWSize;
        configChanged_ = false;

        QString size_str = QString("%1×%2").arg(QString::number(fr.preprocessed.width()),
                                                QString::number(fr.preprocessed.height()));

        // if (downscale_fctr >= 2)
        //{
        // size_str += QString(" /%1").arg(downscale_fctr);
        //}

        emit frameSizeStr(size_str);
    }
    else
    {
        if (config.hasTemporalFiltering)
        {
            smoother_.update(algoImage);
            algoImage = smoother_.outputSpan();
        }

        activeContour_->resetExecutionState(algoImage);
    }

    activeContour_->runCycles(config.cyclesNbr);
    fr.processTimestampNs = FrameClock::nowNs() - startTs;

    exportTemporalFilteredImage(algoImage, config, fr);
    exportContours(fr);

    return fr;
}

void VideoActiveContourThread::exportTemporalFilteredImage(const fluvel_ip::ImageView& algoImage,
                                                           const VideoComputeConfig& config,
                                                           DisplayFrame& fr)
{
    if (!config.hasTemporalFiltering)
        return;

    fr.preprocessed = QImage(algoImage.data(), algoImage.width(), algoImage.height(),
                             static_cast<qsizetype>(3 * algoImage.width()), QImage::Format_RGB888);
}

void VideoActiveContourThread::exportContours(DisplayFrame& fr)
{
    if (activeContour_)
    {
        fr.outerContour = activeContour_->export_l_out();
        fr.innerContour = activeContour_->export_l_in();
    }
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
