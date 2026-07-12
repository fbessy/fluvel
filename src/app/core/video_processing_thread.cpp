// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "region_color_speed_model.hpp"
#include "speed_model.hpp"
#include "video_processing_thread.hpp"

#include "elapsed_timer.hpp"
#include "frame_clock.hpp"
#include "image_adapters.hpp"

namespace fluvel
{

VideoProcessingThread::VideoProcessingThread(QObject* parent)
    : QThread(parent)

{
}

void VideoProcessingThread::submitFrame(const ReceivedFrame& frame)
{
    QMutexLocker locker(&frameMutex_);

    const int next = writeIndex_ ^ 1;

    buffers_[next] = frame;

    writeIndex_ = next;
    hasNewFrame_ = true;

    condition_.wakeOne();
}

void VideoProcessingThread::run()
{
    running_ = true;

    int readIndex = 0;

    while (running_)
    {
        ReceivedFrame cf;

        {
            QMutexLocker locker(&frameMutex_);

            while (!hasNewFrame_ && running_)
            {
                condition_.wait(&frameMutex_);
            }

            if (!running_)
                break;

            readIndex = writeIndex_;

            cf = buffers_[readIndex];

            hasNewFrame_ = false;
        }

        ProcessedFrame df = processFrame(cf.frame);

        df.receiveTimestampNs = cf.receiveTimestampNs;

        emit frameProcessed(df.outerContour.size() + df.innerContour.size());
        emit processedFrameReady(df);
    }
}

QImage VideoProcessingThread::convertFrame(QVideoFrame frame) const
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

QImage VideoProcessingThread::applyDownscale(const QImage& input,
                                                const DownscaleParams& config) const
{
    if (input.isNull())
        return input;

    if (!config.downscaleEnabled)
        return input;

    const int factor = config.downscaleFactor;

    assert(factor == 2 || factor == 4 || factor == 8 || factor == 16);

    return input.scaled(input.width() / factor, input.height() / factor, Qt::IgnoreAspectRatio,
                        Qt::SmoothTransformation);
}

ProcessedFrame VideoProcessingThread::processFrame(const QVideoFrame& frame)
{
    VideoComputeConfig config;
    ImageDisplayMode displayMode;

    {
        QMutexLocker locker(&configMutex_);
        config = config_;
        displayMode = displayMode_;
    }

    ProcessedFrame df;
    QImage inputImage = convertFrame(frame);

    if (inputImage.isNull())
        return df;

    QImage preprocessed = applyDownscale(inputImage, config.downscale);

    if (preprocessed.isNull())
        return df;

    auto algoImage = imageViewFromQImage(preprocessed);

    const auto& algoConfig = config.contourConfig;

    const auto newSize = preprocessed.size();
    const bool needReset = !activeContour_ || configChanged_ || newSize != currentSize_;

    // ------------------------------------------------------------
    // 1) Filtering pipeline
    // ------------------------------------------------------------
    if (config.spatialFilteringEnabled)
    {
        spatialFilter_.apply(algoImage);
        algoImage = spatialFilter_.outputView();
    }

    if (config.temporalFilteringEnabled)
    {
        if (needReset)
            temporalSmoother_.reset(algoImage);
        else
            temporalSmoother_.update(algoImage);

        algoImage = temporalSmoother_.outputView();
    }

    // ------------------------------------------------------------
    // 2) Active contour lifecycle
    // ------------------------------------------------------------
    if (needReset)
    {
        activeContour_ = std::make_unique<fluvel_ip::ActiveContour>(
            fluvel_ip::ContourData(algoImage.width(), algoImage.height(), algoConfig.connectivity),
            std::make_unique<fluvel_ip::RegionColorSpeedModel>(algoConfig.regionParams),
            algoConfig.contourParams);

        currentSize_ = newSize;
        configChanged_ = false;
    }

    // ------------------------------------------------------------
    // 3) Update
    // ------------------------------------------------------------
    if (algoImage.data() && activeContour_)
    {
        activeContour_->update(algoImage);
    }

    fluvel_ip::ElapsedTimer timeSliceBudget;
    timeSliceBudget.start();

    while (!activeContour_->isStopped() && timeSliceBudget.elapsedLessThan(kTimeSliceMs))
    {
        activeContour_->runCycles(1);
    }

    if (displayMode == ImageDisplayMode::Source)
    {
        df.image = inputImage;
    }
    else if (displayMode == ImageDisplayMode::Preprocessed)
    {
        if (config.spatialFilteringEnabled || config.temporalFilteringEnabled)
            exportFilteredImage(algoImage, df);
        else
            df.image = preprocessed;
    }

    exportContours(df);

    df.processTimestampNs = FrameClock::nowNs();

    return df;
}

void VideoProcessingThread::exportFilteredImage(const fluvel_ip::ImageView& algoImage,
                                                   ProcessedFrame& displayFrame)
{
    displayFrame.image = toQImageCopy(algoImage);
}

void VideoProcessingThread::exportContours(ProcessedFrame& displayFrame)
{
    if (activeContour_)
    {
        displayFrame.outerContour = activeContour_->exportOuterBoundary();
        displayFrame.innerContour = activeContour_->exportInnerBoundary();
    }
}

void VideoProcessingThread::stop()
{
    QMutexLocker locker(&frameMutex_);

    running_ = false;

    condition_.wakeAll();
}

void VideoProcessingThread::setAlgoConfig(const VideoComputeConfig& config)
{
    QMutexLocker locker(&configMutex_);
    config_ = config;
    configChanged_ = true;
}

void VideoProcessingThread::setDisplayMode(ImageDisplayMode mode)
{
    QMutexLocker locker(&configMutex_);
    displayMode_ = mode;
    displayModeChanged_ = true;
}

} // namespace fluvel
