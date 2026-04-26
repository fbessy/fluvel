// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file video_active_contour_thread.hpp
 * @brief Dedicated thread for real-time video processing using active contours.
 *
 * This component runs the active contour algorithm on incoming video frames
 * in a separate thread. It handles:
 * - frame acquisition from the capture layer
 * - preprocessing (downscale, filtering)
 * - contour computation
 * - conversion to display-ready data
 *
 * The thread operates continuously and emits processed frames for UI consumption.
 */

#pragma once

#ifndef Q_MOC_RUN
#include "active_contour.hpp"
#include "application_settings_types.hpp"
#include "image_view.hpp"
#include "mean_filter_3x3.hpp"
#include "temporal_mean.hpp"
#endif

#include "frame_data.hpp"

#include <QVideoFrame>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include <atomic>
#include <chrono>

namespace fluvel_app
{

/**
 * @brief Worker thread for video-based active contour processing.
 *
 * This class processes frames asynchronously in a dedicated thread.
 * It maintains an internal pipeline:
 *
 * CapturedFrame → preprocessing → active contour → DisplayFrame
 *
 * Thread-safety is ensured through mutexes and atomic flags.
 *
 * @note Designed for real-time processing with bounded time slices.
 */
class VideoActiveContourThread : public QThread
{
    Q_OBJECT
public:
    /**
     * @brief Construct the processing thread.
     */
    VideoActiveContourThread(QObject* parent);

    /**
     * @brief Submit a new frame for processing.
     *      * The frame may replace previously unprocessed frames depending on timing.
     *      * @param capturedFrame Frame received from capture layer.
     */
    void submitFrame(const CapturedFrame& capturedFrame);

    /**
     * @brief Stop the processing thread.
     */
    void stop();

    /**
     * @brief Update algorithm configuration.
     */
    void setAlgoConfig(const VideoComputeConfig& config);

    /**
     * @brief Set display mode.
     */
    void setDisplayMode(ImageDisplayMode mode);

signals:
    /// Emitted after processing a frame (contour size for diagnostics).
    void frameProcessed(quint64 contourSize);

    /// Emitted when a display-ready frame is available.
    void displayFrameReady(fluvel_app::DisplayFrame displayFrame);

protected:
    /**
     * @brief Main processing loop.
     *      * Continuously waits for new frames and processes them within a time budget.
     */
    void run() override;

private:
    /**
     * @brief Convert a QVideoFrame to QImage.
     */
    QImage convertFrame(QVideoFrame frame) const;

    /**
     * @brief Apply downscaling to input image.
     */
    QImage applyDownscale(const QImage& input, const DownscaleParams& config) const;

    /**
     * @brief Process a single frame through the pipeline.
     */
    DisplayFrame processFrame(const QVideoFrame& inputFrame);

    /**
     * @brief Export processed image to display frame.
     */
    void exportFilteredImage(const fluvel_ip::ImageView& algoImage, DisplayFrame& displayFrame);

    /**
     * @brief Export contours to display frame.
     */
    void exportContours(DisplayFrame& displayFrame);

    /// Maximum processing time slice per iteration.
    static constexpr std::chrono::milliseconds kTimeSliceMs{20};

    /// Current algorithm configuration.
    VideoComputeConfig config_;

    /// Current display mode.
    ImageDisplayMode displayMode_;

    /// Mutex protecting frame access.
    QMutex frameMutex_;

    /// Mutex protecting configuration updates.
    QMutex configMutex_;

    /// Condition variable for frame availability.
    QWaitCondition condition_;

    /// Last received frame.
    CapturedFrame lastCapturedFrame_;

    /// Whether a new frame is available.
    bool frameAvailable_{false};

    /// Thread running flag.
    std::atomic<bool> running_{false};

    /// Flags indicating pending updates.
    bool configChanged_{false};
    bool displayModeChanged_{false};

    /// Active contour instance.
    std::unique_ptr<fluvel_ip::ActiveContour> activeContour_;

    /// Current frame size.
    QSize currentSize_ = {0, 0};

    /// Spatial smoothing filter (3x3 mean).
    fluvel_ip::filter::Mean3x3 spatialFilter_;

    /// Temporal smoothing filter.
    fluvel_ip::filter::TemporalMean temporalSmoother_;

    /// Double buffer for captured frames.
    CapturedFrame buffers_[2];

    /// Index of the write buffer.
    std::atomic<int> writeIndex_{0};

    /// Indicates if a new frame is available.
    std::atomic<bool> hasNewFrame_{false};
};

} // namespace fluvel_app
