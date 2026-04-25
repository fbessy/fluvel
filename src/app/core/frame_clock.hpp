// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file frame_clock.hpp
 * @brief Global monotonic clock for frame timing.
 *
 * This module provides a centralized time reference based on a steady clock.
 * It is used to measure timestamps and durations consistently across the
 * application (e.g. frame capture, processing, display).
 *
 * The clock is initialized once and all subsequent calls return time relative
 * to that origin.
 */

#pragma once

#include <chrono>
#include <cstdint>

namespace fluvel_app
{

/**
 * @brief Global monotonic clock utility.
 *
 * Provides high-resolution timestamps relative to an internal start point.
 * Designed for performance measurement and synchronization within the
 * application pipeline.
 *
 * @note Uses std::chrono::steady_clock (monotonic, not affected by system time changes).
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

} // namespace fluvel_app
