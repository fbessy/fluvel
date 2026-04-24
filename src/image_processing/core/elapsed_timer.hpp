// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <chrono>
#include <type_traits>

namespace fluvel_ip
{

/**
 * @brief Simple utility class to measure elapsed time.
 *
 * This class provides a lightweight wrapper around std::chrono
 * to measure elapsed time since a starting point.
 *
 * It is typically used for:
 * - performance measurements
 * - time-based filtering
 * - diagnostics and profiling
 */
class ElapsedTimer
{
public:
    /// Clock type used internally (steady, monotonic).
    using clock = std::chrono::steady_clock;

    /**
     * @brief Start the timer.
     *
     * Sets the reference time to the current time.
     */
    void start()
    {
        start_ = clock::now();
    }

    /**
     * @brief Restart the timer.
     *
     * Equivalent to calling start().
     */
    void restart()
    {
        start();
    }

    /**
     * @brief Get elapsed time in seconds.
     *
     * @tparam T Floating-point return type (default: double).
     * @return Elapsed time in seconds.
     */
    template <typename T = double>
    T elapsedSec() const
        requires std::is_floating_point_v<T>
    {
        return std::chrono::duration<T>(clock::now() - start_).count();
    }

    /**
     * @brief Get elapsed time in milliseconds.
     *
     * @tparam T Floating-point return type (default: double).
     * @return Elapsed time in milliseconds.
     */
    template <typename T = double>
    T elapsedMs() const
        requires std::is_floating_point_v<T>
    {
        return std::chrono::duration<T, std::milli>(clock::now() - start_).count();
    }

    /**
     * @brief Check if elapsed time is less than a given duration.
     *
     * @tparam Duration Any std::chrono duration type.
     * @param d Duration to compare against.
     * @return True if elapsed time is less than d.
     */
    template <typename Duration>
    bool elapsedLessThan(Duration d) const
    {
        return (clock::now() - start_) < d;
    }

private:
    /// Start time point.
    clock::time_point start_;
};

} // namespace fluvel_ip
