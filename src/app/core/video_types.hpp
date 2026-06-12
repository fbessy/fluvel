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

enum class SourceType
{
    Camera,
    Url,
    File
};

struct SourceConfig
{
    SourceType type{SourceType::Camera};

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
 * @brief Information about a video source.
 *
 * Contains the source selected by the user and the resolved
 * information required to identify it during its lifetime.
 *
 * This information is available before the first frame is received
 * and can therefore be used for startup failures, source errors,
 * stream loss notifications and successful stream initialization.
 */
struct SourceInfo
{
    /**
     * @brief Source type.
     */
    SourceType type{SourceType::Camera};

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