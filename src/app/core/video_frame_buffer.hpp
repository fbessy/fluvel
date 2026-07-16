// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "frame_pipeline.hpp"
#include "video_frame_spool.hpp"

#include <QQueue>

#include <cstddef>
#include <optional>

namespace fluvel
{

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
     * @brief Result of a frame insertion.
     */
    enum class PushStatus
    {
        /// Frame successfully queued.
        Success,

        /// Memory usage exceeded the recommended threshold.
        MemoryWarning,

        /// Memory limit reached. The frame was not queued.
        MemoryLimitExceeded
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
     * @brief Removes all queued frames.
     */
    void clear();

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
     * @brief Returns the total memory occupied by queued frames.
     *
     * @return Memory usage in bytes.
     */
    [[nodiscard]]
    std::size_t queuedBytes() const;

private:
    /**
     * @brief Returns the memory occupied by an image.
     *
     * @param image Image to evaluate.
     *
     * @return Image size in bytes.
     */
    static std::size_t frameSize(const QImage& image);

    QQueue<VideoFrame> queue_;

    std::size_t queuedBytes_{0};

    bool memoryWarningEmitted_{false};

    static constexpr std::size_t kWarningMemoryBytes = 1200ull * 1024 * 1024;

    static constexpr std::size_t kMaxMemoryBytes = 2000ull * 1024 * 1024;

    VideoFrameSpool spool_;

    bool spoolEnabled_{false};
};

} // namespace fluvel