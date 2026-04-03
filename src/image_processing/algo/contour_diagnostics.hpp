// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ac_types.hpp"

#include <cstddef>
#include <vector>

namespace fluvel_ip
{

class ChannelVector
{
public:
    ChannelVector(int v)
    {
        values_ = {v};
    }

    ChannelVector(int r, int g, int b)
    {
        values_ = {r, g, b};
    }

    size_t dimension() const
    {
        return values_.size();
    }

    const std::vector<int>& values() const
    {
        return values_;
    }

private:
    std::vector<int> values_;
};

struct ContourDiagnostics
{
    int stepCount = 0;
    fluvel_ip::PhaseState state{fluvel_ip::PhaseState::Cycle1};
    ChannelVector meanInside{0, 0, 0};
    ChannelVector meanOutside{0, 0, 0};
    fluvel_ip::StoppingStatus stoppingStatus{fluvel_ip::StoppingStatus::None};
    float hausdorffQuantile = 0.f;
    float relativeCentroidDistance = 0.f;
    double elapsedSec = 0.0;
    std::size_t contourSize = 0;
};

inline const char* toString(PhaseState state)
{
    switch (state)
    {
        case PhaseState::Cycle1:
            return "Cycle 1";
        case PhaseState::Cycle2:
            return "Cycle 2";
        case PhaseState::FinalCycle2:
            return "Final Cycle 2";
        case PhaseState::Stopped:
            return "Stopped";
    }

    return "Unknown";
}

inline const char* toString(StoppingStatus status)
{
    switch (status)
    {
        case StoppingStatus::None:
            return "None";
        case StoppingStatus::ListsConverged:
            return "ListsConverged";
        case StoppingStatus::Hausdorff:
            return "Hausdorff";
        case StoppingStatus::MaxIteration:
            return "MaxIteration";
        case StoppingStatus::EmptyListFailure:
            return "EmptyListFailure";
        case StoppingStatus::SpeedModelFailure:
            return "SpeedModelFailure";
    }

    return "Unknown";
}

} // namespace fluvel_ip
