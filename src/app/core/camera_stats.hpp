// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

namespace fluvel_app
{

struct CameraStats
{
    double inputFps = 0.0;
    double processingFps = 0.0;
    double displayFps = 0.0;
    double dropRate = 0.0;
    double avgLatencyMs = 0.0;
    double maxLatencyMs = 0.0;
    double avgContourSize = 0.0;
};

} // namespace fluvel_app
