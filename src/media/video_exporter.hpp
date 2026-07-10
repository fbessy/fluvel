// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "video_exporter_backend.hpp"

#include <memory>

class QImage;

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
     * @brief Adds a frame to the video.
     *
     * Frames are encoded in the order they are received.
     *
     * @param image Source image.
     *
     * @return True on success, false otherwise.
     */
    [[nodiscard]]
    bool addFrame(const QImage& image);

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

private:
    std::unique_ptr<IVideoExporter> exporter_;
};

} // namespace fluvel