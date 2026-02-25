#pragma once

namespace ofeli_app
{

struct CameraStats
{
    double inputFps = 0.0;
    double processingFps = 0.0;
    double displayFps = 0.0;
    double dropRate = 0.0;
    double avgLatencyMs = 0.0;
    double maxLatencyMs = 0.0;
};

} // namespace ofeli_app
