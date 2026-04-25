// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "elapsed_timer.hpp"
#include "frame_data.hpp"

#include <QMutex>
#include <QtGlobal>

namespace fluvel_app
{

/**
 * @brief Computes real-time statistics for the frame processing pipeline.
 *
 * This class tracks frame lifecycle events (capture, processing, display)
 * and computes aggregated statistics over a sliding time window (~1 second).
 *
 * It provides metrics such as:
 * - capture / processing / display FPS
 * - frame drop rate
 * - processing and display latencies
 * - average contour size
 *
 * Thread-safety:
 * - All updates and reads are protected by a mutex
 * - snapshot() returns a stable copy of the last computed values
 *
 * The statistics are updated periodically based on an internal time window.
 */
class FrameStatsView
{
public:
    /**
     * @brief Snapshot of computed statistics.
     *      * Values are averaged over the current time window.
     */
    struct Snapshot
    {
        double capturedFps{0.0};
        double processedFps{0.0};
        double displayedFps{0.0};
        double dropRate{0.0};
        double avgLatencyDisplayMs{0.0};
        double maxLatencyDisplayMs{0.0};
        double avgLatencyProcMs{0.0};
        double avgContourSize{0.0};
    };

    FrameStatsView();

    /**
     * @name Frame events
     * @brief Notify the stats system of frame lifecycle events.
     * @{
     */
    void frameCaptured();
    void frameProcessed(quint64 contourSize);
    void frameDisplayed(const FrameTimestamps& ts);
    /** @} */

    /**
     * @brief Returns a stable snapshot of the current statistics.
     *      * The snapshot is computed over the last time window (~1 second).
     */
    Snapshot snapshot();

    /**
     * @brief Resets all statistics.
     */
    void reset();

private:
    /**
     * @brief Updates the current statistics window.
     *      * Must be called with mutex locked.
     */
    void updateWindowLocked();

    QMutex mutex_;

    fluvel_ip::ElapsedTimer windowTimer_;

    /// Duration of the statistics window (~1 second).
    static constexpr std::chrono::milliseconds kWindowMs{1000};

    // snapshot précédent (pour affichage stable)
    Snapshot lastSnapshot_{};

    // compteurs fenêtre courante
    quint64 capturedFrames_{0};
    quint64 processedFrames_{0};
    quint64 displayedFrames_{0};
    quint64 droppedFrames_{0};

    quint64 contourSizeSum_{0};

    // latence fenêtre
    double latencySumDisplayMs_{0.0};
    double latencyMaxDisplayMs_{0.0};
    double latencySumProcMs_{0.0};
};

} // namespace fluvel_app
