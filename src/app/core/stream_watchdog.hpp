// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <cstdint>

namespace fluvel
{

/**
 * @brief Monitors the continuity of a video stream.
 *
 * StreamWatchdog tracks the arrival time of valid video frames and detects
 * when an active stream stops producing frames for too long.
 *
 * Before monitoring becomes active, the watchdog waits for the stream to
 * remain stable for a minimum duration and number of frames. This avoids
 * reporting a stream loss while the source is still starting or recovering.
 *
 * The class does not perform any scheduling and does not stop the stream
 * itself. The caller is responsible for periodically checking hasTimedOut()
 * and reacting appropriately.
 */
class StreamWatchdog
{
public:
    /**
     * @brief Resets the watchdog to its initial unarmed state.
     *
     * All timing and stabilization information is discarded.
     */
    void reset();

    /**
     * @brief Notifies the watchdog that a valid frame has been received.
     *
     * The timestamp must use the same monotonic time base as the value passed
     * to hasTimedOut() and frameAgeNs().
     *
     * @param timestampNs Frame reception timestamp, in nanoseconds.
     */
    void frameReceived(int64_t timestampNs);

    /**
     * @brief Returns whether stream-loss monitoring is currently active.
     *
     * @return true if the watchdog has completed its stabilization phase.
     */
    [[nodiscard]] bool isArmed() const noexcept;

    /**
     * @brief Checks whether the stream has stopped producing frames.
     *
     * @param nowNs Current timestamp, in nanoseconds.
     * @return true if the age of the last valid frame exceeds the stream-loss
     *         timeout.
     */
    [[nodiscard]] bool hasTimedOut(int64_t nowNs) const noexcept;

    /**
     * @brief Returns the age of the most recently received valid frame.
     *
     * @param nowNs Current timestamp, in nanoseconds.
     * @return Frame age, in nanoseconds.
     */
    [[nodiscard]] int64_t frameAgeNs(int64_t nowNs) const noexcept;

private:
    void arm() noexcept;

    /**
     * @brief Minimum number of consecutive valid frames required before
     *        the watchdog can be armed.
     *
     * @details
     * The watchdog is armed only when both this frame-count threshold and
     * @c kStabilizationNs have been reached.
     *
     * A higher value delays activation of stream-loss detection and can be
     * useful for sources with unstable startup behaviour.
     */
    static constexpr int kMinStableFrames{5};

    /**
     * @brief Minimum duration for which valid frames must be received before
     *        the watchdog can be armed.
     *
     * @details
     * The watchdog is armed only when both this duration and
     * @c kMinStableFrames have been reached.
     *
     * A longer duration makes startup detection more tolerant but delays
     * activation of stream-loss monitoring.
     */
    static constexpr int64_t kStabilizationNs{500'000'000};

    /**
     * @brief Maximum allowed age of the last valid frame before the stream
     *        is considered lost.
     *
     * @details
     * A shorter timeout detects stream interruptions sooner but may be more
     * sensitive to temporary frame delays. A longer timeout is more tolerant
     * of transient interruptions but delays stream-loss detection.
     */
    static constexpr int64_t kStreamLossTimeoutNs{2'000'000'000};

    int64_t lastFrameTimestampNs_{0};
    int64_t stableSinceNs_{0};
    int stableFrameCount_{0};

    bool armed_{false};
    bool stabilizing_{false};
};

} // namespace fluvel