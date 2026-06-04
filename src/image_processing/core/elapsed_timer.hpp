// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <chrono>
#include <type_traits>

#include <cassert>

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
        started_ = true;
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
    [[nodiscard]]
    T elapsedSec() const
        requires std::is_floating_point_v<T>
    {
        assert(started_);
        return std::chrono::duration<T>(clock::now() - start_).count();
    }

    /**
     * @brief Get elapsed time in milliseconds.
     *
     * @tparam T Floating-point return type (default: double).
     * @return Elapsed time in milliseconds.
     */
    template <typename T = double>
    [[nodiscard]]
    T elapsedMs() const
        requires std::is_floating_point_v<T>
    {
        assert(started_);
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
    [[nodiscard]]
    bool elapsedLessThan(Duration d) const
    {
        assert(started_);
        return (clock::now() - start_) < d;
    }

    [[nodiscard]]
    bool isStarted() const noexcept
    {
        return started_;
    };

private:
    /// Start time point.
    clock::time_point start_{};

    /// Indicates whether the timer has been started.
    bool started_{false};
};

} // namespace fluvel_ip
