// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <cstddef>

namespace fluvel
{

/**
 * @brief Behavior when the recording buffer reaches its capacity.
 */
enum class BufferOverflowPolicy
{
    /**
     * @brief Stop the recording.
     *
     * Recording stops when the recording buffer becomes full.
     */
    StopRecording,

    /**
     * @brief Use a circular recording buffer.
     *
     * The oldest buffered frames are discarded to make room for newly
     * buffered frames.
     */
    Circular
};

/**
 * @brief Recording buffer configuration.
 *
 * Defines the limits used by the recording buffer to manage memory and
 * temporary disk storage during video recording.
 *
 * Video frames are initially buffered in RAM. Once @ref maxRamUsage is
 * reached, additional frames are transparently written to temporary storage.
 *
 * Recording continues until @ref maxDiskUsage is reached. The action taken
 * when this limit is exceeded is controlled by @ref overflowPolicy.
 */
struct RecordingBufferSettings
{
    /**
     * @brief Maximum amount of RAM used for buffering video frames.
     *
     * Once this limit is reached, newly buffered frames are temporarily
     * written to disk.
     */
    std::size_t maxRamUsage{1024ull * 1024 * 1024}; // 1 GiB

    /**
     * @brief Maximum amount of temporary disk space used for buffering.
     *
     * This limit applies only to the temporary storage used while recording.
     */
    std::size_t maxDiskUsage{5ull * 1024 * 1024 * 1024}; // 5 GiB

    /**
     * @brief Behavior when the disk usage limit is reached.
     */
    BufferOverflowPolicy overflowPolicy{BufferOverflowPolicy::StopRecording};
};

} // namespace fluvel