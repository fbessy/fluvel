// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

namespace fluvel_app
{

struct CameraStats
{
    double capturedFps = 0.0;
    double processedFps = 0.0;
    double displayedFps = 0.0;
    double dropRate = 0.0;
    double avgLatencyDisplayMs = 0.0;
    double maxLatencyDisplayMs = 0.0;
    double avgLatencyProcMs = 0.0;
    double avgContourSize = 0.0;
};

} // namespace fluvel_app
