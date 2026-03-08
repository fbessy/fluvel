// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "frame_clock.hpp"

namespace fluvel_app
{

QElapsedTimer FrameClock::timer;

void FrameClock::FrameClock::init()
{
    if (!timer.isValid())
        timer.start();
}

qint64 FrameClock::FrameClock::nowNs()
{
    return timer.nsecsElapsed();
}

} // namespace fluvel_app
