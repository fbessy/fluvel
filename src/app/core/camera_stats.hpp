#pragma once

namespace ofeli_app {

struct CameraStats
{
    float inputFps = 0.0;
    float processingFps = 0.0;
    float displayFps = 0.0;
    float dropRate = 0.0;
    float avgLatencyMs = 0.0;
    float maxLatencyMs = 0.0;
};

} // namespace
