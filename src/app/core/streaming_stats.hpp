// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

namespace fluvel
{

/**
 * @brief Aggregated statistics for video streaming and processing.
 *
 * StreamingStats stores runtime metrics related to video acquisition,
 * processing and display performance.
 *
 * It is primarily used for diagnostics, monitoring and user interface
 * reporting.
 *
 * Metrics include frame rates, latency measurements, frame drop rate
 * and contour statistics.
 */
struct StreamingStats
{
    /**
     * @brief Frames received from the active video source (FPS).
     */
    double receivedFps{0.0};

    /**
     * @brief Frames processed by the processing pipeline (FPS).
     */
    double processedFps{0.0};

    /**
     * @brief Frames displayed to the user (FPS).
     */
    double displayedFps{0.0};

    /**
     * @brief Ratio of received frames that were not displayed.
     */
    double dropRate{0.0};

    /**
     * @brief Average latency from frame reception to display (ms).
     */
    double avgLatencyDisplayMs{0.0};

    /**
     * @brief Maximum observed latency from frame reception to display (ms).
     */
    double maxLatencyDisplayMs{0.0};

    /**
     * @brief Average processing time per frame (ms).
     */
    double avgLatencyProcMs{0.0};

    /**
     * @brief Average contour size (number of contour points).
     */
    double avgContourSize{0.0};
};

} // namespace fluvel