#pragma once

namespace ofeli_app {

struct CameraStats
{
    float inputFps = 0.f;
    float processingFps = 0.f;
    float displayFps = 0.f;
    float dropRate = 0.f;
    float avgLatencyMs = 0.f;
    float maxLatencyMs = 0.f;
};

} // namespace
