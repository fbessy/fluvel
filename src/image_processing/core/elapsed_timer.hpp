// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <chrono>
#include <type_traits>

namespace fluvel_ip
{

class ElapsedTimer
{
public:
    using clock = std::chrono::steady_clock;

    void start()
    {
        start_ = clock::now();
    }

    void restart()
    {
        start();
    }

    template <typename T = double>
    T elapsedSec() const
        requires std::is_floating_point_v<T>
    {
        return std::chrono::duration<T>(clock::now() - start_).count();
    }

    template <typename T = double>
    T elapsedMs() const
        requires std::is_floating_point_v<T>
    {
        return std::chrono::duration<T, std::milli>(clock::now() - start_).count();
    }

    template <typename Duration> bool elapsedLessThan(Duration d) const
    {
        return (clock::now() - start_) < d;
    }

private:
    clock::time_point start_;
};

} // namespace fluvel_ip
