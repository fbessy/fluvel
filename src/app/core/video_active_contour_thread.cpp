// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_active_contour_thread.hpp"
#include "region_color_ac.hpp"
#include "speed_model.hpp"

#include "elapsed_timer.hpp"
#include "frame_clock.hpp"
#include "image_adapters.hpp"

namespace fluvel_app
{

VideoActiveContourThread::VideoActiveContourThread(QObject* parent)
    : QThread(parent)

{
}

void VideoActiveContourThread::submitFrame(const CapturedFrame& frame)
{
    int wi = writeIndex_.load(std::memory_order_relaxed);
    int next = 1 - wi;

    buffers_[next] = frame;

    writeIndex_.store(next, std::memory_order_release);
    hasNewFrame_.store(true, std::memory_order_release);
}

void VideoActiveContourThread::run()
{
    running_.store(true);

    int readIndex = 0;

    while (running_.load())
    {
        if (hasNewFrame_.load(std::memory_order_acquire))
        {
            int wi = writeIndex_.load(std::memory_order_acquire);

            if (wi != readIndex)
            {
                readIndex = wi;

                CapturedFrame cf = buffers_[readIndex];

                DisplayFrame df = processFrame(cf.frame);

                df.receiveTimestampNs = cf.receiveTimestampNs;

                emit frameProcessed(df.outerContour.size() + df.innerContour.size());
                emit displayFrameReady(df);
            }

            hasNewFrame_.store(false, std::memory_order_release);
        }
        else
        {
            QThread::usleep(200);
        }
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
        QMutexLocker locker(&configMutex_);
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

        activeContour_ = std::make_unique<fluvel_ip::ActiveContour>(
            fluvel_ip::ContourData(algoImage.width(), algoImage.height(), algoConfig.connectivity),
            std::make_unique<fluvel_ip::RegionColorSpeedModel>(algoConfig.regionAcConfig),
            algoConfig.acConfig);

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
    }

    if (algoImage.data() != nullptr && activeContour_)
        activeContour_->update(algoImage);

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
    QMutexLocker locker(&configMutex_);
    config_ = config;
    configChanged_ = true;
}

void VideoActiveContourThread::setDisplayMode(ImageDisplayMode mode)
{
    QMutexLocker locker(&configMutex_);
    displayMode_ = mode;
    displayModeChanged_ = true;
}

} // namespace fluvel_app
