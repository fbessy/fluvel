// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "frame_pipeline.hpp"
#include "recording_buffer_settings.hpp"
#include "video_frame_spool.hpp"

#include <QQueue>

#include <cstddef>
#include <cstdint>
#include <optional>

namespace fluvel
{

/**
 * @brief Storage location of a buffered video frame.
 *
 * This enumeration identifies where a buffered frame is currently
 * stored.
 */
enum class StorageType
{
    /**
     * @brief Frame stored in main memory.
     */
    Memory,

    /**
     * @brief Frame stored in the temporary spool file.
     */
    Disk
};

/**
 * @brief Video frame stored by the recording buffer.
 *
 * A buffered frame may either reside in memory or in the temporary
 * spool file. When the frame is stored in memory, @ref frame contains
 * the complete video frame. Otherwise, @ref location identifies the
 * corresponding frame stored on disk.
 */
struct BufferedFrame
{
    /**
     * @brief Storage location of the buffered frame.
     */
    StorageType storage{StorageType::Memory};

    /**
     * @brief Video frame stored in memory.
     *
     * This member is valid only when @ref storage is equal to
     * StorageType::Memory.
     */
    VideoFrame frame;

    /**
     * @brief Location of the frame in the spool file.
     *
     * This member is valid only when @ref storage is equal to
     * StorageType::Disk.
     */
    FrameLocation location;
};

/**
 * @brief Buffers video frames before encoding.
 *
 * This class stores video frames awaiting encoding and tracks the
 * associated memory usage.
 *
 * The current implementation keeps all frames in memory. Future
 * implementations may transparently spill frames to disk while
 * preserving the same public API.
 */
class VideoFrameBuffer
{
public:
    /**
     * @brief Result of inserting a video frame into the recording buffer.
     */
    enum class PushStatus
    {
        /**
         * @brief Frame successfully queued.
         *
         * The frame was stored either in memory or in the temporary storage.
         */
        Success,

        /**
         * @brief Recording buffer switched to temporary storage.
         *
         * The frame has been accepted, but the recording buffer has reached
         * its configured RAM limit and is now using temporary storage.
         *
         * This status is emitted only once per recording session.
         */
        TemporaryStorageActivated,

        /**
         * @brief Recording buffer capacity exceeded.
         *
         * The frame could not be stored because both the in-memory buffer
         * and the temporary storage have reached their configured limits,
         * or because temporary storage is unavailable.
         */
        BufferLimitExceeded
    };

    /**
     * @brief Constructs an empty video frame buffer.
     *
     * The temporary spool is initialized during construction.
     */
    VideoFrameBuffer();

    /**
     * @brief Queues a video frame.
     *
     * The frame image is deep-copied so that it remains valid
     * independently of the caller.
     *
     * @param frame Frame to enqueue.
     *
     * @return Result of the insertion.
     */
    [[nodiscard]]
    PushStatus push(const VideoFrame& frame);

    /**
     * @brief Removes and returns the oldest queued frame.
     *
     * @return The next frame if available, or @c std::nullopt
     *         when the buffer is empty.
     */
    [[nodiscard]]
    std::optional<VideoFrame> pop();

    /**
     * @brief Removes all queued frames.
     */
    void clear();

    /**
     * @brief Removes all buffered frames and deletes the temporary spool file.
     *
     * Clears the in-memory queue, releases any temporary storage used on disk,
     * and resets the internal buffer state. The temporary spool file is removed
     * and will be recreated automatically when a new recording session starts.
     */
    void removeTemporaryStorage();

    /**
     * @brief Checks whether the buffer is empty.
     *
     * @return @c true if no frame is queued.
     */
    [[nodiscard]]
    bool empty() const;

    /**
     * @brief Returns the number of queued frames.
     *
     * @return Number of queued frames.
     */
    [[nodiscard]]
    std::size_t queuedFrames() const;

    /**
     * @brief Returns the total amount of memory currently occupied by queued frames.
     *      * @return Total memory usage in bytes.
     */
    [[nodiscard]]
    uint64_t queuedBytes() const;

    /**
     * @brief Returns the amount of RAM currently occupied by queued frames.
     *      * @return RAM usage in bytes.
     */
    [[nodiscard]]
    uint64_t queuedMemoryBytes() const;

    /**
     * @brief Returns the amount of temporary storage currently occupied by queued frames.
     *      * @return Temporary storage usage in bytes.
     */
    [[nodiscard]]
    uint64_t queuedDiskBytes() const;

private:
    /**
     * @brief Returns the memory occupied by an image.
     *
     * @param image Image to evaluate.
     *
     * @return Image size in bytes.
     */
    static uint64_t frameSize(const QImage& image);

    /**
     * @brief Recording buffer configuration.
     */
    RecordingBufferSettings settings_{};

    /**
     * @brief Buffered video frames waiting to be encoded.
     */
    QQueue<BufferedFrame> queue_;

    /**
     * @brief Temporary storage used when the RAM buffer is full.
     */
    VideoFrameSpool spool_;

    /**
     * @brief Amount of RAM currently used by buffered frames.
     */
    uint64_t queuedBytes_{0};

    /**
     * @brief Amount of temporary storage currently used by buffered frames.
     */
    uint64_t queuedDiskBytes_{0};

    /**
     * @brief Indicates whether temporary storage is currently being used.
     */
    bool usingTemporaryStorage_{false};
};

} // namespace fluvel