// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QCamera>
#include <QCameraFormat>
#include <QMediaPlayer>
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
};

/**
 * @brief Metadata describing a media source.
 *
 * Contains information provided by the media backend that may become
 * available asynchronously after the source has been opened.
 *
 * Unlike StreamingInfo, these properties describe the media itself
 * rather than the currently received video frames.
 */
struct MediaInfo
{
    /**
     * @brief Human-readable media title.
     *
     * Typically extracted from embedded metadata when available.
     * Empty when no suitable title is provided by the media.
     */
    QString title;

    /**
     * @brief Indicates whether random seeking is supported.
     *
     * When true, playback position can be changed through the media
     * timeline. Live streams are typically not seekable.
     */
    bool seekable{false};

    /**
     * @brief Media duration in milliseconds.
     *
     * Zero when unknown, unavailable, or not applicable.
     */
    qint64 durationMs{0};

    /**
     * @brief Whether the media contains audio.
     */
    bool hasAudio{false};

    /**
     * @brief Source frame rate reported by the backend.
     */
    double frameRate{0.0};
};

/**
 * @brief Information describing a camera error.
 *
 * This structure captures the context of a camera-related error,
 * including the source that triggered the error and the controller
 * state at the time the error occurred.
 *
 * It is intended to be propagated through signals and used by the UI
 * or higher-level components to present diagnostics and error messages.
 */
struct CameraErrorInfo
{
    /**
     * @brief Camera error reported by Qt Multimedia.
     */
    QCamera::Error error;

    /**
     * @brief Human-readable error description.
     */
    QString errorString;

    /**
     * @brief Source associated with the error.
     */
    SourceInfo sourceInfo{};

    /**
     * @brief Controller state when the error occurred.
     */
    StreamingState state{StreamingState::Stopped};
};

/**
 * @brief Information describing a media player error.
 *
 * This structure captures the context of a media playback error,
 * including the source that triggered the error and the controller
 * state at the time the error occurred.
 *
 * It is intended to be propagated through signals and used by the UI
 * or higher-level components to present diagnostics and error messages.
 */
struct MediaPlayerErrorInfo
{
    /**
     * @brief Media player error reported by Qt Multimedia.
     */
    QMediaPlayer::Error error;

    /**
     * @brief Human-readable error description.
     */
    QString errorString;

    /**
     * @brief Source associated with the error.
     */
    SourceInfo sourceInfo{};

    /**
     * @brief Controller state when the error occurred.
     */
    StreamingState state{StreamingState::Stopped};
};

} // namespace fluvel