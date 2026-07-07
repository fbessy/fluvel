// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

class QImage;

namespace fluvel
{

class VideoExportSettings;

/**
 * @brief Video exporter backend interface.
 *
 * A video exporter backend is responsible for encoding a sequence
 * of images into a video file.
 *
 * The implementation may rely on FFmpeg or any other encoding
 * library, but must expose the same high-level API.
 */
class IVideoExporter
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IVideoExporter() = default;

    /**
     * @brief Opens a new video.
     *
     * @param settings Video export settings.
     *
     * @return True on success.
     */
    virtual bool open(const VideoExportSettings& settings) = 0;

    /**
     * @brief Encodes a video frame.
     *
     * Frames are encoded in the order they are received.
     *
     * @param image Source image.
     *
     * @return True on success.
     */
    virtual bool addFrame(const QImage& image) = 0;

    /**
     * @brief Finalizes the video.
     *
     * Flushes any buffered frames and closes the output file.
     *
     * @return True on success.
     */
    virtual bool close() = 0;

    /**
     * @brief Returns whether the exporter is opened.
     */
    virtual bool isOpen() const = 0;
};

} // namespace fluvel