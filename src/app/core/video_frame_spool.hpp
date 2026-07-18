// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "frame_pipeline.hpp"

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
     * @brief Returns whether the spool file is opened.
     *
     * @return @c true if the spool is opened.
     */
    [[nodiscard]]
    bool isOpen() const;

    /**
     * @brief Returns the current size of the spool file in bytes.
     *
     * @return Size of the spool file in bytes, or 0 if the spool file is not open.
     */
    [[nodiscard]] quint64 size() const;

    /**
     * @brief Resets the spool for a new recording session.
     *
     * Removes all stored frames, recreates the temporary spool file and resets
     * the internal write position. The spool remains open and ready to accept
     * new frames.
     *
     * @return @c true on success, @c false otherwise.
     */
    [[nodiscard]]
    bool reset();

    /**
     * @brief Removes the temporary spool file.
     *
     * Closes and deletes the temporary spool file, then resets the internal
     * write position. The spool can be reopened later by calling open().
     *
     * @return @c true on success, @c false otherwise.
     */
    [[nodiscard]]
    bool remove();

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

private:
    QTemporaryDir temporaryDirectory_;

    QFile spoolFile_;

    quint64 writeOffset_{0};
};

} // namespace fluvel