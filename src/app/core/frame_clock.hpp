// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <chrono>
#include <cstdint>

namespace fluvel
{

/**
 * @brief Global monotonic clock utility.
 *
 * FrameClock provides a centralized monotonic time reference used
 * throughout the application for frame capture, processing and display
 * timing measurements.
 *
 * All timestamps are expressed relative to a common initialization point,
 * ensuring consistent duration and latency measurements across the pipeline.
 *
 * @note Uses std::chrono::steady_clock and is not affected by system time changes.
 */
class FrameClock
{
public:
    using clock = std::chrono::steady_clock;

    /**
     * @brief Initialize the clock reference point.
     *
     * Must be called once before using nowNs() or nowSec().
     * Subsequent calls have no effect.
     */
    static void init();

    /**
     * @brief Get current time in nanoseconds since initialization.
     *
     * @return Elapsed time in nanoseconds.
     */
    static int64_t nowNs();

    /**
     * @brief Get current time in seconds since initialization.
     *
     * @return Elapsed time in seconds.
     */
    static double nowSec();

private:
    /// Reference start time.
    static inline clock::time_point start_{};

    /// Whether the clock has been initialized.
    static inline bool initialized_{false};
};

} // namespace fluvel
