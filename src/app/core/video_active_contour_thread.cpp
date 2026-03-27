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

void VideoActiveContourThread::submitFrame(const CapturedFrame& capturedFrame)
{
    {
        QMutexLocker locker(&frameMutex_);
        lastCapturedFrame_ = capturedFrame;
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

        CapturedFrame cf = lastCapturedFrame_;
        frameAvailable_ = false;
        locker.unlock();

        DisplayFrame df = processFrame(cf.frame);

        df.receiveTimestampNs = cf.receiveTimestampNs;

        quint64 contourSize = static_cast<quint64>(df.outerContour.size() + df.innerContour.size());

        emit frameProcessed(contourSize);

        emit displayFrameReady(df);

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
    VideoComputeConfig config;
    ImageDisplayMode displayMode;

    {
        QMutexLocker locker(&frameMutex_);
        config = config_;
        displayMode = displayMode_;
    }

    DisplayFrame df;
    QImage inputImage = convertFrame(frame);

    if (inputImage.isNull())
        return df;

    QImage preprocessed = applyDownscale(inputImage, config.downscale);

    if (preprocessed.isNull())
        return df;

    auto algoImage = image_view_from_qimage(preprocessed);
    const auto& algoConfig = config.algo;

    const auto newWSize = preprocessed.size();

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

    QElapsedTimer timeSliceBudgetMs;
    timeSliceBudgetMs.start();

    while (!activeContour_->isStopped() && timeSliceBudgetMs.elapsed() < kTimeSliceMs)
    {
        activeContour_->runCycles(1);
    }

    if (displayMode == ImageDisplayMode::Source)
    {
        df.image = inputImage;
    }
    else if (displayMode == ImageDisplayMode::Preprocessed)
    {
        if (config.hasTemporalFiltering)
            exportTemporalFilteredImage(algoImage, df);
        else
            df.image = preprocessed;
    }

    exportContours(df);

    df.processTimestampNs = FrameClock::nowNs();

    return df;
}

void VideoActiveContourThread::exportTemporalFilteredImage(const fluvel_ip::ImageView& algoImage,
                                                           DisplayFrame& displayFrame)
{
    displayFrame.image =
        QImage(algoImage.data(), algoImage.width(), algoImage.height(),
               static_cast<qsizetype>(3 * algoImage.width()), QImage::Format_RGB888);
}

void VideoActiveContourThread::exportContours(DisplayFrame& displayFrame)
{
    if (activeContour_)
    {
        displayFrame.outerContour = activeContour_->export_l_out();
        displayFrame.innerContour = activeContour_->export_l_in();
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

void VideoActiveContourThread::setDisplayMode(ImageDisplayMode mode)
{
    QMutexLocker locker(&frameMutex_);
    displayMode_ = mode;
    displayModeChanged_ = true;
}

} // namespace fluvel_app
