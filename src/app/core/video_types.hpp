// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QCameraFormat>
#include <QSize>
#include <QString>
#include <QUrl>
#include <QVideoFrameFormat>

namespace fluvel
{

/**
 * @brief Supported video source types.
 */
enum class SourceType
{
    None,
    Camera,
    Media
};

/**
 * @brief User-selected source configuration.
 *
 * Contains the parameters required to start a video source.
 *
 * This structure represents the desired source state as configured
 * from the user interface before the source is started.
 *
 * Depending on the source type, only a subset of the fields is used:
 * - cameraId and cameraFormat for camera sources
 * - url for media sources
 */
struct SourceConfig
{
    SourceType type{SourceType::None};

    QByteArray cameraId;
    QCameraFormat cameraFormat;

    QUrl url;
};

/**
 * @brief Current streaming state.
 */
enum class StreamingState
{
    Stopped,  ///< No active stream
    Starting, ///< Camera initialization in progress
    Streaming ///< Frames are being received
};

/**
 * @brief Runtime information about a configured video source.
 *
 * Extends SourceConfig with information resolved during source
 * initialization, such as the source description.
 *
 * This information is available before the first frame is received
 * and can therefore be used for startup failures, source errors,
 * stream loss notifications and successful stream initialization.
 */
struct SourceInfo
{
    /**
     * @brief Source type.
     *
     * SourceType::None indicates that no source is currently active.
     */
    SourceType type{SourceType::None};

    /**
     * @brief Unique camera identifier.
     *
     * Only valid when @ref type is SourceType::Camera.
     */
    QByteArray deviceId;

    /**
     * @brief Active camera format.
     *
     * Only valid when @ref type is SourceType::Camera.
     */
    QCameraFormat deviceFormat;

    /**
     * @brief Source URL.
     *
     * Can represent:
     * - a local file (file://)
     * - an HTTP/HTTPS stream
     * - an RTSP stream
     * - any other supported media source
     */
    QUrl sourceUrl;

    /**
     * @brief Human-readable source description.
     *
     * Examples:
     * - Camera device name
     * - Stream name or host
     * - Video file name
     */
    QString description;

    /**
     * @brief Checks whether this active source matches the given source configuration.
     *
     * Compares the source type and the parameters that uniquely identify the source:
     * - camera device and format for camera sources
     * - URL for media sources
     *
     * The human-readable description is ignored.
     *
     * @param config Source configuration to compare against.
     *
     * @return true if both represent the same source configuration, false otherwise.
     */
    bool matches(const SourceConfig& config) const;
};

/**
 * @brief Runtime information about an active video stream.
 *
 * Extends SourceInfo with media characteristics discovered
 * after the stream has successfully started.
 */
struct StreamingInfo
{
    /**
     * @brief Source information.
     */
    SourceInfo source;

    /**
     * @brief Current frame dimensions in pixels.
     */
    QSize frameSize;

    /**
     * @brief Pixel format of received video frames.
     */
    QVideoFrameFormat::PixelFormat pixelFormat{QVideoFrameFormat::Format_Invalid};

    /**
     * @brief Source frame rate reported by the backend.
     */
    double sourceFrameRate{0.0};

    /**
     * @brief Indicates whether random seeking is supported.
     */
    bool seekable{false};

    /**
     * @brief Media duration in milliseconds.
     *
     * Zero when unknown or not applicable.
     */
    qint64 durationMs{0};
};

} // namespace fluvel