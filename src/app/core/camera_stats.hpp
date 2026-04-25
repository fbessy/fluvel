// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file camera_stats.hpp
 * @brief Runtime statistics for camera processing pipeline.
 *
 * This structure aggregates performance metrics related to video capture,
 * processing, and display. It is typically used for diagnostics, monitoring,
 * and UI display.
 */

#pragma once

namespace fluvel_app
{

/**
 * @brief Aggregated statistics for camera streaming and processing.
 *
 * All values are expressed in real-time metrics and updated periodically.
 */
struct CameraStats
{
    double capturedFps{0.0};  ///< Frames captured from camera (FPS)
    double processedFps{0.0}; ///< Frames processed by the algorithm (FPS)
    double displayedFps{0.0}; ///< Frames displayed to the user (FPS)

    double dropRate{0.0}; ///< Ratio of frames captured but not displayed (end-to-end drop rate)

    double avgLatencyDisplayMs{0.0}; ///< Average latency from capture to display (ms)
    double maxLatencyDisplayMs{0.0}; ///< Maximum observed display latency (ms)

    double avgLatencyProcMs{0.0}; ///< Average processing time per frame (ms)

    double avgContourSize{0.0}; ///< Average contour size (number of points)
};

} // namespace fluvel_app
