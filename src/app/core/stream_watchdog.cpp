// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "stream_watchdog.hpp"

#include <cassert>

namespace fluvel
{

void StreamWatchdog::reset()
{
    armed_ = false;
    stabilizing_ = false;

    lastFrameTimestampNs_ = 0;
    stableSinceNs_ = 0;
    stableFrameCount_ = 0;
}

void StreamWatchdog::frameReceived(int64_t timestampNs)
{
    lastFrameTimestampNs_ = timestampNs;

    if (armed_)
        return;

    ++stableFrameCount_;

    if (!stabilizing_)
    {
        stabilizing_ = true;
        stableSinceNs_ = timestampNs;
        return;
    }

    if (stableFrameCount_ >= kMinStableFrames && timestampNs - stableSinceNs_ >= kStabilizationNs)
    {
        arm();
    }
}

bool StreamWatchdog::isArmed() const noexcept
{
    return armed_;
}

bool StreamWatchdog::hasTimedOut(int64_t nowNs) const noexcept
{
    assert(!(armed_ && stabilizing_));

    return armed_ && frameAgeNs(nowNs) > kStreamLossTimeoutNs;
}

int64_t StreamWatchdog::frameAgeNs(int64_t nowNs) const noexcept
{
    return nowNs - lastFrameTimestampNs_;
}

void StreamWatchdog::arm() noexcept
{
    armed_ = true;
    stabilizing_ = false;

    stableSinceNs_ = 0;
    stableFrameCount_ = 0;
}

} // namespace fluvel