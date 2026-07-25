// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "frame_pipeline.hpp"

#include <QFile>
#include <QTemporaryDir>

#include <optional>
#include <vector>

namespace fluvel
{

/**
 * @brief Location of a video frame stored in the temporary spool storage.
 *
 * Identifies the spool segment and the byte offset of a video frame,
 * allowing it to be restored later.
 */
struct FrameLocation
{
    /**
     * @brief Index of the spool segment containing the frame.
     *
     * This value corresponds to an index in the internal spool segment
     * collection.
     */
    int segment{0};

    /**
     * @brief Byte offset of the frame within the spool segment.
     */
    quint64 offset{0};
};

/**
 * @brief Segmented temporary spool storage for video frames.
 *
 * Video frames are stored uncompressed in one or more temporary spool
 * segments when the in-memory recording buffer reaches its configured
 * capacity.
 *
 * Frames can later be restored transparently and forwarded to the video
 * exporter.
 *
 * The spool segments are created in a temporary directory and are
 * automatically removed when the spool is destroyed.
 */
class VideoFrameSpool
{
public:
    /**
     * @brief Destroys the video frame spool.
     *
     * Any open spool segments are automatically closed.
     */
    ~VideoFrameSpool();

    VideoFrameSpool() = default;
    VideoFrameSpool(const VideoFrameSpool&) = delete;
    VideoFrameSpool& operator=(const VideoFrameSpool&) = delete;

    /**
     * @brief Sets the maximum temporary disk space used by the spool.
     *
     * Updates the maximum storage capacity of the spool and recomputes its
     * internal segment layout.
     *
     * @param bytes Maximum spool size in bytes.
     */
    void setMaximumSize(quint64 bytes);

    /**
     * @brief Creates and opens the temporary spool storage.
     *
     * A temporary directory is created for the current recording session
     * and the spool segments are initialized.
     *
     * @return @c true on success, @c false otherwise.
     */
    [[nodiscard]]
    bool open();

    /**
     * @brief Closes all spool segments.
     *
     * Any buffered data is flushed before each segment is closed.
     */
    void close();

    /**
     * @brief Returns whether the spool is open.
     *
     * @return @c true if all spool segments have been successfully opened.
     */
    [[nodiscard]]
    bool isOpen() const;

    /**
     * @brief Returns the amount of data currently stored in the spool.
     *
     * @return Number of bytes currently occupied by buffered frames.
     */
    [[nodiscard]] quint64 size() const;

    /**
     * @brief Resets the temporary spool storage.
     *
     * Removes all temporary data previously written to the spool and
     * reinitializes all spool segments for subsequent read and write
     * operations.
     *
     * After a successful call, the spool remains open and can immediately be
     * reused for a new recording session.
     *
     * @return @c true if the spool was successfully reset, or @c false otherwise.
     */
    bool reset();

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
     * @brief Releases a frame previously stored in the spool.
     *
     * Decrements the reference count of the segment containing the frame.
     * When the last referenced frame of a segment has been released,
     * the segment becomes available for reuse.
     *
     * @param location Location previously returned by write().
     */
    void release(const FrameLocation& location);

private:
    /**
     * @brief Temporary spool segment.
     *
     * A spool segment stores video frames sequentially in a temporary file.
     * Once all buffered frames belonging to the segment have been released,
     * the segment can be reset and reused for subsequent recordings.
     */
    struct SpoolSegment
    {
        /**
         * @brief Temporary file backing this spool segment.
         *
         * The file is stored through a unique pointer because QFile is neither
         * copyable nor movable. Using dynamic allocation allows SpoolSegment to
         * remain movable while preserving exclusive ownership of the underlying
         * temporary file.
         */
        std::unique_ptr<QFile> file;

        /**
         * @brief Current write position in the segment.
         *
         * Frames are appended sequentially starting from the beginning of the
         * file. The offset is reset to zero when the segment is recycled.
         */
        quint64 writeOffset{0};

        /**
         * @brief Number of buffered frames currently referencing this segment.
         *
         * This counter is incremented whenever a frame is written to the
         * segment and decremented when the frame leaves the recording buffer.
         * A value of zero indicates that the segment no longer contains any
         * referenced frames and may safely be reset and reused.
         */
        int refCount{0};
    };

    /**
     * @brief Prepares a spool segment for writing.
     *
     * If the current segment still has enough free space, it is kept.
     * Otherwise, the next available reusable segment is searched in cyclic
     * order. A reusable segment is a segment whose reference count is zero.
     * Before being reused, the segment is truncated and its write position is
     * reset to the beginning of the file.
     *
     * @param requiredSize Number of bytes that must fit into the selected
     *        segment.
     * @return @c true if a writable segment is available, @c false otherwise.
     */
    bool prepareWriteSegment(quint64 requiredSize);

    QTemporaryDir temporaryDirectory_;

    std::vector<SpoolSegment> segments_;

    quint64 maximumSize_{0};

    quint64 segmentSize_{0};
    int segmentCount_{0};

    int currentWriteSegment_{0};
};

} // namespace fluvel