// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "elapsed_timer.hpp"
#include "frame_pipeline.hpp"

#include <QMutex>
#include <QtGlobal>

namespace fluvel
{

/**
 * @brief Computes real-time statistics for the video processing pipeline.
 *
 * This class tracks frame lifecycle events (reception, processing and display)
 * and computes aggregated statistics over a sliding time window (~1 second).
 *
 * It provides metrics such as:
 * - reception / processing / display FPS
 * - frame drop rate
 * - processing and display latencies
 * - average contour size
 *
 * Thread-safety:
 * - All updates and reads are protected by a mutex
 * - snapshot() returns a stable copy of the last computed values
 *
 * Statistics are periodically recomputed using an internal sliding time window.
 */
class FrameStatsCollector
{
public:
    /**
     * @brief Statistics computed over the current time window.
     */
    struct Snapshot
    {
        double receivedFps{0.0};
        double processedFps{0.0};
        double displayedFps{0.0};
        double dropRate{0.0};
        double avgLatencyDisplayMs{0.0};
        double maxLatencyDisplayMs{0.0};
        double avgLatencyProcMs{0.0};
        double avgContourSize{0.0};
    };

    FrameStatsCollector();

    /**
     * @name Frame events
     * @brief Notify the stats system of frame lifecycle events.
     * @{
     */
    void frameReceived();
    void frameProcessed(quint64 contourSize);
    void frameDisplayed(const FrameTimestamps& ts);
    /** @} */

    /**
     * @brief Returns the most recently computed statistics snapshot.
     *
     * The returned values are averaged over the current statistics window.
     */
    Snapshot snapshot();

    /**
     * @brief Resets all statistics.
     */
    void reset();

private:
    /**
     * @brief Updates the current statistics window.
     *
     * Must be called with mutex locked.
     */
    void updateWindowLocked();

    QMutex mutex_;

    fluvel_ip::ElapsedTimer windowTimer_;

    /// Duration of the statistics window (~1 second).
    static constexpr std::chrono::milliseconds kWindowMs{1000};

    /// Last computed snapshot used for stable reporting.
    Snapshot lastSnapshot_{};

    /// Frame counters for the current statistics window.
    quint64 receivedFrames_{0};
    quint64 processedFrames_{0};
    quint64 displayedFrames_{0};
    quint64 droppedFrames_{0};

    quint64 contourSizeSum_{0};

    /// Latency accumulators for the current statistics window.
    double latencySumDisplayMs_{0.0};
    double latencyMaxDisplayMs_{0.0};
    double latencySumProcMs_{0.0};
};

} // namespace fluvel
