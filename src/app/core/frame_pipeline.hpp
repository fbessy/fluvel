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

#include <optional>

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
 * @brief Frame representation produced by the processing stage.
 *
 * Contains the image representation associated with the processing
 * stage together with the extracted contours represented using core
 * Fluvel types.
 *
 * The image is not necessarily a processed image. Depending on the
 * selected representation mode, it may contain the source image or
 * an explicitly processed representation. The contours, however,
 * always originate from the processing stage.
 *
 * This structure is typically produced by the processing thread before
 * conversion to a display-specific representation.
 */
struct ProcessedFrame
{
    /**
     * @brief Image representation associated with the processed frame.
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
struct DisplayFrame
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

/**
 * @brief Frame ready for video encoding.
 *
 * Contains the rendered image together with the presentation timestamp
 * used by the video exporter.
 */
struct VideoFrame
{
    /**
     * @brief Rendered image.
     */
    QImage image;

    /**
     * @brief Presentation timestamp in nanoseconds.
     *
     * When specified, the frame is exported using explicit presentation
     * timestamps. Otherwise, constant frame rate timing is used.
     */
    std::optional<int64_t> presentationTimestampNs;
};

} // namespace fluvel
