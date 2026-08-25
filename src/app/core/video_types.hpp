// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file video_types.hpp
 * @brief Common data structures exchanged between the video controller,
 *        user interface and streaming components.
 */

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
 * @brief Configuration of a camera video source.
 *
 * Contains the parameters required to select and configure a camera
 * before the source is started.
 */
struct CameraConfig
{
    /**
     * @brief Unique identifier of the camera device.
     */
    QByteArray deviceId;

    /**
     * @brief Camera format requested for the source.
     */
    QCameraFormat deviceFormat;
};

/**
 * @brief Configuration of a media video source.
 *
 * Contains the parameters required to open a media source.
 */
struct MediaSourceConfig
{
    /**
     * @brief Media source URL.
     *
     * Can represent a local file, an HTTP/HTTPS stream, an RTSP stream,
     * or any other media source supported by the backend.
     */
    QUrl sourceUrl;
};

/**
 * @brief Runtime information about a configured camera source.
 *
 * Contains information resolved by the camera backend during source
 * initialization.
 */
struct CameraInfo
{
    /**
     * @brief Unique identifier of the camera device.
     */
    QByteArray deviceId;

    /**
     * @brief Active camera format.
     */
    QCameraFormat deviceFormat;

    /**
     * @brief Human-readable camera description.
     */
    QString description;

    /**
     * @brief Checks whether this camera matches the given configuration.
     *
     * The comparison checks the camera identifier and camera format.
     * The human-readable description is ignored.
     *
     * @param config Camera configuration to compare against.
     *
     * @return true if the camera matches the configuration, false otherwise.
     */
    bool matches(const CameraConfig& config) const;
};

/**
 * @brief Runtime information about a configured media source.
 *
 * Contains information resolved during media source initialization.
 */
struct MediaSourceInfo
{
    /**
     * @brief Media source URL.
     */
    QUrl sourceUrl;

    /**
     * @brief Human-readable source description.
     */
    QString description;

    /**
     * @brief Checks whether this media source matches the given configuration.
     *
     * @param config Media source configuration to compare against.
     *
     * @return true if the source URL matches the configuration, false otherwise.
     */
    bool matches(const MediaSourceConfig& config) const;
};

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
 * Depending on the source type, only the corresponding configuration
 * is used:
 * - @ref CameraConfig for camera sources
 * - @ref MediaSourceConfig for media sources
 */
struct SourceConfig
{
    SourceType type{SourceType::None};

    CameraConfig camera;
    MediaSourceConfig media;
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
 * Contains the source type and the runtime information resolved by the
 * corresponding video source during initialization.
 *
 * Depending on the source type, only the corresponding information is used:
 * - @ref CameraInfo for camera sources
 * - @ref MediaSourceInfo for media sources
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
     * @brief Runtime information for a camera source.
     *
     * Only valid when @ref type is SourceType::Camera.
     */
    CameraInfo camera;

    /**
     * @brief Runtime information for a media source.
     *
     * Only valid when @ref type is SourceType::Media.
     */
    MediaSourceInfo media;

    /**
     * @brief Checks whether this active source matches the given source configuration.
     *
     * The comparison is delegated to the source-specific information:
     * - @ref CameraInfo::matches for camera sources
     * - @ref MediaSourceInfo::matches for media sources
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