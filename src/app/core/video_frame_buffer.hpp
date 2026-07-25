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

struct RecorderStats;

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
     * @brief Frame stored in the temporary spool storage.
     */
    Disk
};

/**
 * @brief Buffers video frames before encoding.
 *
 * Frames are stored in memory until the configured RAM limit is reached.
 * Additional frames may then be written to segmented temporary spool storage.
 *
 * The public API remains independent of the underlying storage location.
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
     * @brief Location of the frame in the temporary spool storage.
     *
     * This member is valid only when @ref storage is equal to
     * StorageType::Disk.
     */
    FrameLocation location;

    /**
     * @brief Size of the buffered frame, in bytes.
     *
     * This value is cached regardless of the storage location. It allows
     * the recording buffer to update its memory and disk usage counters
     * without reading the frame back from the temporary spool storage.
     */
    uint64_t sizeBytes{0};

    /**
     * @brief Presentation timestamp of the buffered frame, in nanoseconds.
     *
     * This timestamp is retained regardless of whether the frame is stored
     * in memory or in the temporary spool storage. It is used to compute the
     * amount of recording history currently available in the buffer.
     */
    std::optional<int64_t> presentationTimestampNs;
};

/**
 * @brief Buffers video frames for asynchronous video recording.
 *
 * This class stores video frames produced by the application before they
 * are encoded by the recording thread. It also tracks the associated
 * memory usage and optionally spills frames to temporary spool storage when
 * required.
 *
 * The public API is independent of the underlying storage, allowing frames
 * to be kept in memory or transparently written to a segmented spool on
 * disk.
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
         * The frame was stored either in memory or in the temporary spool storage.
         */
        Success,

        /**
         * @brief Recording buffer switched to temporary spool storage.
         *
         * The frame has been accepted, but the recording buffer has reached
         * its configured RAM limit and is now using temporary spool storage.
         *
         * This status is emitted only once per recording session.
         */
        TemporaryStorageActivated,

        /**
         * @brief Recording buffer capacity exceeded.
         *
         * The frame could not be stored because both the in-memory buffer
         * and the temporary spool storage have reached their configured limits,
         * or because temporary spool storage is unavailable.
         */
        BufferLimitExceeded
    };

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
     * @brief Resets the recording buffer to its initial state.
     *
     * Clears all buffered frames, resets the associated temporary spool
     * storage, clears the memory and disk usage counters, and restores the
     * initial buffering state.
     *
     * This function is typically called before starting a new recording
     * session.
     */
    void reset();

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
     *
     * @return Total memory usage in bytes.
     */
    [[nodiscard]]
    uint64_t queuedBytes() const;

    /**
     * @brief Returns the amount of RAM currently occupied by queued frames.
     *
     * @return RAM usage in bytes.
     */
    [[nodiscard]]
    uint64_t queuedMemoryBytes() const;

    /**
     * @brief Returns the amount of temporary spool storage currently occupied by queued frames.
     *
     * @return Temporary storage usage in bytes.
     */
    [[nodiscard]]
    uint64_t queuedDiskBytes() const;

    /**
     * @brief Updates the recording buffer settings.
     *
     * The new settings are applied to subsequent recording operations. Existing
     * buffered frames are not modified.
     *
     * @param settings New recording buffer settings.
     */
    void setSettings(const RecordingBufferSettings& settings);

    /**
     * @brief Fills a structure with the current runtime statistics of the buffer.
     *
     * All values are captured from the current state of the buffer and can be
     * safely displayed by the user interface.
     *
     * @param[out] stats Structure receiving the current buffer statistics.
     */
    void fillStats(RecorderStats& stats) const;

private:
    /**
     * @brief Adds a video frame using the stop recording overflow policy.
     *
     * The frame is first stored in memory while sufficient RAM is available.
     * Once the RAM limit is reached, subsequent frames are written to the
     * temporary spool storage until the disk usage limit is reached.
     *
     * If neither memory nor temporary spool storage can accommodate the frame,
     * PushStatus::BufferLimitExceeded is returned.
     *
     * @param frame Video frame to buffer.
     *
     * @return The result of the buffering operation.
     */
    PushStatus pushStopRecording(const VideoFrame& frame);

    /**
     * @brief Pushes a frame using the discard-oldest overflow policy.
     *
     * The frame is first stored in memory whenever possible, then in temporary
     * spool storage. If the recording buffer is full, the oldest buffered frames
     * are discarded until enough space is available for the new frame, allowing
     * the recording to continue.
     *
     * @param frame Frame to enqueue.
     * @return Status of the push operation.
     */
    PushStatus pushDiscardOldest(const VideoFrame& frame);

    /**
     * @brief Attempts to buffer a video frame.
     *
     * The frame is first stored in memory whenever sufficient RAM is
     * available. Otherwise, it is written to the temporary spool storage if
     * enough disk space remains.
     *
     * This function never discards existing frames and never changes the
     * configured overflow policy. It simply attempts to store the frame
     * using the currently available resources.
     *
     * @param frame Video frame to buffer.
     * @param frameBytes Size of the frame in bytes.
     *
     * @return Result of the buffering operation.
     */
    PushStatus tryPush(const VideoFrame& frame, quint64 frameBytes);

    /**
     * @brief Stores a video frame in memory.
     *
     * Creates a deep copy of the frame image, appends the frame to the
     * recording queue, and updates the memory usage counters.
     *
     * @param frame Video frame to buffer.
     * @param frameBytes Size of the frame in bytes.
     *
     * @return PushStatus::Success.
     */
    PushStatus pushToMemory(const VideoFrame& frame, qsizetype frameBytes);

    /**
     * @brief Stores a video frame in the temporary spool storage.
     *
     * Writes the frame to the temporary spool storage, appends its metadata to the
     * recording queue, and updates the temporary spool storage usage counters.
     *
     * If the temporary spool storage is used for the first time during the
     * current recording session, PushStatus::TemporaryStorageActivated is
     * returned.
     *
     * @param frame Video frame to buffer.
     * @param frameBytes Size of the frame in bytes.
     *
     * @return The result of the buffering operation.
     */
    PushStatus pushToDisk(const VideoFrame& frame, qsizetype frameBytes);

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
     * @brief Temporary spool storage used when the RAM buffer is full.
     */
    VideoFrameSpool spool_;

    /**
     * @brief Amount of RAM currently used by buffered frames.
     */
    uint64_t queuedBytes_{0};

    /**
     * @brief Amount of temporary spool storage currently used by buffered frames.
     */
    uint64_t queuedDiskBytes_{0};

    /**
     * @brief Indicates whether temporary spool storage is currently being used.
     */
    bool usingTemporaryStorage_{false};

    /**
     * @brief Total number of frames submitted to the recording buffer.
     *
     * This counter is incremented for every frame passed to the recording
     * buffer, regardless of whether the frame is eventually encoded or
     * discarded.
     */
    uint64_t submittedFrames_{0};

    /**
     * @brief Total number of buffered frames discarded because of buffer overflow.
     *
     * This counter is incremented only when the configured overflow policy
     * removes the oldest buffered frame to make room for a newer one.
     */
    uint64_t discardedFrames_{0};
};

} // namespace fluvel