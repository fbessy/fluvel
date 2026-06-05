// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "temporal_mean.hpp"
#include "image_conversions.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace fluvel_ip::filter
{

void TemporalMean::reset(const ImageView& firstFrame)
{
    assert(firstFrame.channels() >= 3);

    hasTemporalHistory_ = false;

    if (output_.width() != firstFrame.width() || output_.height() != firstFrame.height() ||
        output_.format() != ImageFormat::Bgr32)
    {
        output_ = ImageOwner(firstFrame.width(), firstFrame.height(), ImageFormat::Bgr32);
    }

    accum_.resize(firstFrame.width(), firstFrame.height());

    for (int y = 0; y < accum_.height(); ++y)
    {
        for (int x = 0; x < accum_.width(); ++x)
        {
            const Rgb_uc color = firstFrame.atPixelRgb(x, y);

            accum_.at(x, y) = Rgb_f{static_cast<float>(color.red), static_cast<float>(color.green),
                                    static_cast<float>(color.blue)};
        }
    }
    outputNeedsRefresh_ = true;

    const int pixelsCount = accum_.width() * accum_.height();
    constexpr int targetSamples = 200000;

    samplingStep_ = std::max(1, int(std::sqrt(float(pixelsCount) / targetSamples)));

    motionRatioFiltered_ = 1.f;

    warmupTimer_.start();
}

// ------------------------------------------------------------
// UPDATE
// ------------------------------------------------------------
void TemporalMean::update(const ImageView& frame)
{
    if (accum_.empty())
    {
        reset(frame);
        return;
    }

    if (frame.width() != accum_.width() || frame.height() != accum_.height())
    {
        reset(frame);
        return;
    }

    const float motion = computeMotion(frame);

    float deltaSeconds = 0.033f;

    if (hasTemporalHistory_)
        deltaSeconds = deltaFrameTimer_.elapsedSec<float>();

    deltaSeconds = std::clamp(deltaSeconds, 0.001f, 0.5f);
    deltaFrameTimer_.restart();

    if (hasTemporalHistory_)
        updateNoiseEstimate(motion, deltaSeconds);
    else
        noiseEstimate_ = motion;

    const float alpha = computeAdaptiveAlpha(motion, deltaSeconds);

    applyTemporalSmoothing(frame, alpha);

    outputNeedsRefresh_ = true;
    hasTemporalHistory_ = true;
}

float TemporalMean::computeMotion(const ImageView& frame) const
{
    float motion = 0.f;
    int count = 0;

    for (int y = 0; y < accum_.height(); y += samplingStep_)
    {
        for (int x = 0; x < accum_.width(); x += samplingStep_)
        {
            const Rgb_uc current = frame.atPixelRgb(x, y);
            const Rgb_f& accumPixel = accum_.at(x, y);

            const float dr = float(current.red) - accumPixel.red;
            const float dg = float(current.green) - accumPixel.green;
            const float db = float(current.blue) - accumPixel.blue;

            motion += dr * dr + dg * dg + db * db;
            ++count;
        }
    }

    return std::sqrt(motion / (3.f * count));
}

// ------------------------------------------------------------
// Noise estimation (FPS independent)
// ------------------------------------------------------------
void TemporalMean::updateNoiseEstimate(float motion, float deltaSeconds)
{
    constexpr float tauInit = 0.3f; // startup convergence
    constexpr float tauUp = 0.3f;   // noise increase
    constexpr float tauDown = 2.0f; // noise decrease
    constexpr float warmupDurationSeconds = 0.5f;

    // Prevent large scene changes from instantly inflating
    // the estimated noise floor.
    const float maxAllowed = noiseEstimate_ * 3.f;
    motion = std::min(motion, maxAllowed);

    // Select the time constant depending on whether
    // the estimated noise is increasing or decreasing.
    float tau = tauDown;

    if (motion > noiseEstimate_)
        tau = tauUp;

    // Faster convergence during the startup phase.
    if (warmupTimer_.elapsedSec<float>() < warmupDurationSeconds)
        tau = tauInit;

    const float alpha = 1.f - std::exp(-deltaSeconds / tau);

    noiseEstimate_ += alpha * (motion - noiseEstimate_);
}

float TemporalMean::computeAdaptiveAlpha(float motion, float deltaSeconds)
{
    // --------------------------------------------------------
    // Motion normalized by noise
    // --------------------------------------------------------
    constexpr float epsilon = 1e-6f;

    const float motionRatio = motion / (noiseEstimate_ + epsilon);

    // Smooth the motion ratio to avoid rapid oscillations.
    constexpr float tauRatio = 0.1f;

    const float alphaRatio = 1.f - std::exp(-deltaSeconds / tauRatio);

    // Smooth the normalized motion estimate to avoid
    // rapid fluctuations of the adaptive response.
    // This prevents abrupt changes of the temporal
    // smoothing coefficient between successive frames.
    motionRatioFiltered_ += alphaRatio * (motionRatio - motionRatioFiltered_);

    // --------------------------------------------------------
    // Map motion ratio to time constant
    // --------------------------------------------------------
    constexpr float tauMin = 0.02f; // very reactive
    constexpr float tauMax = 0.2f;  // stable

    constexpr float ratioLow = 1.1f;
    constexpr float ratioHigh = 1.6f;

    float t = 0.f;

    if (motionRatioFiltered_ > ratioLow)
        t = (motionRatioFiltered_ - ratioLow) / (ratioHigh - ratioLow);

    t = std::clamp(t, 0.f, 1.f);

    const float tCurve = std::sqrt(t);

    const float tau = tauMax - tCurve * (tauMax - tauMin);

    // --------------------------------------------------------
    // Convert time constant to EWMA coefficient
    // --------------------------------------------------------
    return 1.f - std::exp(-deltaSeconds / tau);
}

void TemporalMean::applyTemporalSmoothing(const ImageView& frame, float alpha)
{
    const int channels = frame.channels();

    for (int y = 0; y < accum_.height(); ++y)
    {
        const uint8_t* row = frame.row(y);

        for (int x = 0; x < accum_.width(); ++x)
        {
            const int idx = x * channels;

            float r, g, b;
            frame.readPixelRgb(row, idx, r, g, b);

            Rgb_f& accumPixel = accum_.at(x, y);

            accumPixel.red += alpha * (r - accumPixel.red);
            accumPixel.green += alpha * (g - accumPixel.green);
            accumPixel.blue += alpha * (b - accumPixel.blue);
        }
    }
}

// ------------------------------------------------------------
// OUTPUT
// ------------------------------------------------------------
void TemporalMean::updateOutput()
{
    if (!outputNeedsRefresh_)
        return;

    convertRgbFToBgr32(accum_, output_);

    outputNeedsRefresh_ = false;
}

ImageView TemporalMean::outputView()
{
    updateOutput();
    return output_.view();
}

const ImageOwner& TemporalMean::output()
{
    updateOutput();
    return output_;
}

} // namespace fluvel_ip
