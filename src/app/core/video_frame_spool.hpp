// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "frame_pipeline.hpp"
#include "video_frame_header.hpp"

#include <QFile>
#include <QTemporaryDir>

#include <optional>

namespace fluvel
{

/**
 * @brief Location of a video frame stored in the spool file.
 *
 * This structure identifies the position of a frame within the spool
 * file so that it can later be restored.
 */
struct FrameLocation
{
    /**
     * @brief Byte offset of the frame in the spool file.
     */
    quint64 offset{0};
};

/**
 * @brief Temporary storage for video frames.
 *
 * Video frames are stored uncompressed in a temporary spool file when
 * the in-memory recording buffer reaches its configured capacity.
 *
 * Frames can later be restored transparently and forwarded to the video
 * exporter.
 *
 * The spool file is created in a temporary directory and is automatically
 * removed when the spool is destroyed.
 */
class VideoFrameSpool
{
public:
    /**
     * @brief Constructs a video frame spool.
     */
    VideoFrameSpool();

    /**
     * @brief Destroys the video frame spool.
     */
    ~VideoFrameSpool();

    VideoFrameSpool(const VideoFrameSpool&) = delete;
    VideoFrameSpool& operator=(const VideoFrameSpool&) = delete;

    /**
     * @brief Opens the spool file.
     *
     * A temporary directory is created for the current recording session
     * and a spool file is opened inside it.
     *
     * @return @c true on success, @c false otherwise.
     */
    [[nodiscard]]
    bool open();

    /**
     * @brief Closes the spool file.
     *
     * Buffered data is flushed before the file is closed.
     */
    void close();

    /**
     * @brief Removes all stored frames.
     *
     * The spool file remains opened.
     *
     * @return @c true on success, @c false otherwise.
     */
    [[nodiscard]]
    bool clear();

    /**
     * @brief Stores a video frame in the spool.
     *
     * The frame is written using an internal uncompressed binary format.
     *
     * @param frame Frame to store.
     *
     * @return Location of the stored frame on success,
     *         or @c std::nullopt on failure.
     */
    [[nodiscard]]
    std::optional<FrameLocation> write(const VideoFrame& frame);

    /**
     * @brief Restores a previously stored video frame.
     *
     * @param location Location returned by write().
     *
     * @return Restored frame, or @c std::nullopt on failure.
     */
    [[nodiscard]]
    std::optional<VideoFrame> read(const FrameLocation& location);

    /**
     * @brief Returns whether the spool file is opened.
     *
     * @return @c true if the spool is opened.
     */
    [[nodiscard]]
    bool isOpen() const;

private:
    QTemporaryDir temporaryDirectory_;

    QFile spoolFile_;
};

} // namespace fluvel