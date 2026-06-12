// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file frame_pipeline.hpp
 * @brief Data structures representing frames at different stages of the
 * video processing pipeline.
 *
 * This module defines the data exchanged between the different stages of the
 * video pipeline:
 * - source acquisition (camera, stream or file)
 * - processing (algorithms and contour extraction)
 * - UI rendering
 *
 * Each structure carries timestamps to support latency measurement,
 * diagnostics and performance monitoring.
 */

#pragma once

#include "contour_types.hpp"

#include <QImage>
#include <QVideoFrame>
#include <QtCore/qglobal.h>

namespace fluvel
{

/**
 * @brief Frame received from the active video source.
 *
 * Represents a raw frame delivered by the multimedia backend,
 * together with the timestamp at which it was received by the
 * application.
 */
struct ReceivedFrame
{
    /**
     * @brief Raw frame provided by the multimedia backend.
     */
    QVideoFrame frame;

    /**
     * @brief Timestamp when the frame was received (ns).
     */
    int64_t receiveTimestampNs{0};
};

/**
 * @brief Frame after processing, before UI conversion.
 *
 * Contains the processed image together with extracted contours
 * represented using core Fluvel types.
 *
 * This structure is typically produced by the processing thread.
 */
struct DisplayFrame
{
    /**
     * @brief Processed image.
     */
    QImage image;

    /**
     * @brief Outer contour (core representation).
     */
    fluvel_ip::Contour outerContour;

    /**
     * @brief Inner contour (core representation).
     */
    fluvel_ip::Contour innerContour;

    /**
     * @brief Timestamp when the frame was received (ns).
     */
    int64_t receiveTimestampNs{0};

    /**
     * @brief Timestamp when processing completed (ns).
     */
    int64_t processTimestampNs{0};
};

/**
 * @brief Frame ready for UI rendering.
 *
 * Contains display-ready data, including contours converted
 * to Qt-specific types.
 *
 * This structure is intended for direct consumption by the UI layer.
 */
struct UiFrame
{
    /**
     * @brief Display image.
     */
    QImage image;

    /**
     * @brief Outer contour (Qt representation).
     */
    QVector<QPointF> outerContour;

    /**
     * @brief Inner contour (Qt representation).
     */
    QVector<QPointF> innerContour;

    /**
     * @brief Timestamp when the frame was received (ns).
     */
    int64_t receiveTimestampNs{0};

    /**
     * @brief Timestamp when processing completed (ns).
     */
    int64_t processTimestampNs{0};
};

/**
 * @brief Timestamps associated with a frame lifecycle.
 *
 * Used to measure latency throughout the pipeline:
 * - frame reception
 * - processing
 * - display
 */
struct FrameTimestamps
{
    /**
     * @brief Timestamp when the frame was received (ns).
     */
    int64_t receiveTimestampNs{0};

    /**
     * @brief Timestamp when processing completed (ns).
     */
    int64_t processTimestampNs{0};

    /**
     * @brief Timestamp when the frame was displayed (ns).
     */
    int64_t displayTimestampNs{0};
};

} // namespace fluvel
