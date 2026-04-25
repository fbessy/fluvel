// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file frame_data.hpp
 * @brief Data structures representing frames at different stages of the pipeline.
 *
 * This module defines the data exchanged between the different stages of the
 * video processing pipeline:
 * - capture (raw frames from camera)
 * - processing (algorithm + contours)
 * - UI (display-ready data)
 *
 * Each structure carries timestamps to enable latency measurement and diagnostics.
 */

#pragma once

#include "contour_types.hpp"

#include <QImage>
#include <QVideoFrame>
#include <QtTypes>

namespace fluvel_app
{

/**
 * @brief Frame received from the camera capture layer.
 *
 * Represents a raw frame as delivered by Qt multimedia, along with the
 * timestamp at which it was received by the application.
 */
struct CapturedFrame
{
    QVideoFrame frame;             ///< Raw frame from camera
    int64_t receiveTimestampNs{0}; ///< Timestamp when frame was received (ns)
};

/**
 * @brief Frame after processing, before UI conversion.
 *
 * Contains the processed image along with extracted contours in the
 * core (fluvel_ip) representation.
 *
 * This structure is typically produced by the processing thread.
 */
struct DisplayFrame
{
    QImage image; ///< Processed image

    fluvel_ip::ExportedContour outerContour; ///< Outer contour (core representation)
    fluvel_ip::ExportedContour innerContour; ///< Inner contour (core representation)

    int64_t receiveTimestampNs{0}; ///< Timestamp of capture (ns)
    int64_t processTimestampNs{0}; ///< Timestamp after processing (ns)
};

/**
 * @brief Frame ready for UI rendering.
 *
 * Contains display-ready data, including contours converted to Qt types.
 * This structure is intended for direct consumption by the UI layer.
 */
struct UiFrame
{
    QImage image; ///< Display image

    QVector<QPointF> outerContour; ///< Outer contour (Qt representation)
    QVector<QPointF> innerContour; ///< Inner contour (Qt representation)

    int64_t receiveTimestampNs{0}; ///< Timestamp of capture (ns)
    int64_t processTimestampNs{0}; ///< Timestamp after processing (ns)
};

/**
 * @brief Timestamps associated with a frame lifecycle.
 *
 * Used to measure latency across the pipeline:
 * - capture → processing → display
 */
struct FrameTimestamps
{
    int64_t receiveTimestampNs{0}; ///< Capture timestamp (ns)
    int64_t processTimestampNs{0}; ///< Processing completion timestamp (ns)
    int64_t displayTimestampNs{0}; ///< Display timestamp (ns)
};

} // namespace fluvel_app
