// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file recording_types.hpp
 * @brief Common types used by the video recording subsystem.
 */

#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace fluvel
{

/**
 * @brief Recording organization mode.
 *
 * Defines how recorded video is written to disk.
 *
 * - SingleFile: the entire recording is stored in a single video file.
 * - Circular: the recording is split into successive video segments. When the
 *   configured retention limit is reached, the oldest segments are
 *   automatically removed.
 *
 * @note The recording mode is independent of the internal frame buffering
 *       strategy used to absorb temporary encoding delays.
 */
enum class RecordingMode
{
    /**
     * Store the recording in a single video file.
     */
    SingleFile,

    /**
     * Store the recording as successive video segments and automatically
     * remove the oldest segments according to the configured retention
     * settings.
     */
    Circular
};

/**
 * @brief Internal state of the video recorder.
 */
enum class RecorderState
{
    /// No recording session is active.
    Stopped,

    /// Frames are accepted and encoded asynchronously.
    Recording,

    /// Recording has stopped accepting new frames and the queue is being drained.
    Draining
};

/**
 * @brief Runtime statistics of the video recorder.
 */
struct RecorderStats
{
    /**
     * @brief Number of frames currently waiting for encoding.
     */
    std::size_t queuedFrames{0};

    /**
     * @brief Amount of memory currently used by queued frames, in bytes.
     */
    uint64_t queuedMemoryBytes{0};

    /**
     * @brief Amount of temporary storage currently used by queued frames, in bytes.
     */
    uint64_t queuedDiskBytes{0};

    /**
     * @brief Rate at which frames are submitted to the recorder.
     */
    double inputFps{0.0};

    /**
     * @brief Average rate at which frames are encoded.
     */
    double encodingFps{0.0};

    /**
     * @brief Duration of recording data currently retained in the recorder's
     * buffering resources.
     *
     * This corresponds to the amount of recorded media that has not yet been
     * written or discarded. The exact meaning depends on the recorder implementation.
     */
    std::chrono::milliseconds retainedDuration{};

    /**
     * @brief Estimated maximum duration of recording data that can be retained by
     * the recorder's buffering resources.
     *
     * This value is an approximation based on the current recorder configuration
     * and available buffering resources. It may change as the recording bitrate
     * changes.
     */
    std::optional<std::chrono::milliseconds> estimatedMaxRetainedDuration{};

    /**
     * @brief Total duration of the recording accumulated so far.
     *
     * This corresponds to the duration of the recorded output if recording stopped
     * immediately. It includes both the portion already written and the portion
     * still retained by the recorder's buffering resources.
     */
    std::chrono::milliseconds recordedDuration{};

    /**
     * @brief Estimated maximum duration of the recorded output.
     *
     * This corresponds to the maximum duration that the recorded output can reach
     * under the current buffering constraints.
     *
     * This value is an approximation based on the current recording settings and
     * available buffering resources. It may change as the recording bitrate changes.
     */
    std::optional<std::chrono::milliseconds> estimatedMaxRecordedDuration{};

    /**
     * @brief Total number of frames discarded because of buffer overflow.
     *
     * This counter is incremented only when the recording overflow policy
     * discards the oldest buffered frame to make room for a new one.
     */
    uint64_t discardedFrames{0};

    /**
     * @brief Total number of frames submitted for recording.
     */
    uint64_t submittedFrames{0};
};

} // namespace fluvel