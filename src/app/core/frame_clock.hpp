// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <chrono>
#include <cstdint>

namespace fluvel_app
{

class FrameClock
{
public:
    using clock = std::chrono::steady_clock;

    static void init();
    static int64_t nowNs();
    static double nowSec();

private:
    static inline clock::time_point start_{};
    static inline bool initialized_{false};
};

} // namespace fluvel_app
