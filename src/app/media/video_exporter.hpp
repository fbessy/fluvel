// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "video_exporter_backend.hpp"

#include <memory>

class VideoFrame;

namespace fluvel
{

class VideoExportSettings;

/**
 * @brief High-level video exporter.
 *
 * This class provides a simple backend-independent API to export
 * a sequence of images into a video.
 *
 * The actual encoding backend (FFmpeg, etc.) is hidden behind
 * an internal implementation interface.
 */
class VideoExporter
{
public:
    /**
     * @brief Constructs a video exporter.
     */
    VideoExporter();

    /**
     * @brief Destroys the video exporter.
     */
    ~VideoExporter();

    VideoExporter(const VideoExporter&) = delete;
    VideoExporter& operator=(const VideoExporter&) = delete;

    VideoExporter(VideoExporter&&) noexcept;
    VideoExporter& operator=(VideoExporter&&) noexcept;

    /**
     * @brief Opens a new video.
     *
     * This function must be called before addFrame().
     *
     * @param settings Video export settings.
     *
     * @return True on success, false otherwise.
     */
    [[nodiscard]]
    bool open(const VideoExportSettings& settings);

    /**
     * @brief Encodes a video frame.
     *
     * Frames are encoded in the order they are received.
     *
     * Depending on the selected timestamp mode, the presentation timestamp
     * contained in the frame may be ignored and generated automatically.
     *
     * @param frame Video frame to encode.
     * @return @c true on success, @c false otherwise.
     */
    [[nodiscard]]
    bool addFrame(const VideoFrame& frame);

    /**
     * @brief Finalizes the video.
     *
     * Flushes the encoder, writes the trailer and closes
     * the output file.
     *
     * @return True on success, false otherwise.
     */
    [[nodiscard]]
    bool close();

    /**
     * @brief Returns whether the exporter is currently opened.
     *
     * @return True if a video is currently opened.
     */
    [[nodiscard]]
    bool isRecording() const;

    /**
     * @brief Returns the video codecs available for export.
     *
     * The available codecs depend on the active export backend and the
     * encoders available on the current system.
     *
     * @return Available video codecs.
     */
    QList<VideoCodec> availableCodecs() const;

private:
    std::unique_ptr<IVideoExporter> exporter_;
};

} // namespace fluvel