// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "frame_clock.hpp"

namespace fluvel_app
{

void FrameClock::init()
{
    if (!initialized_)
    {
        start_ = clock::now();
        initialized_ = true;
    }
}

int64_t FrameClock::nowNs()
{
    if (!initialized_)
        init();

    return std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now() - start_).count();
}

double FrameClock::nowSec()
{
    return nowNs() * 1e-9;
}

} // namespace fluvel_app
