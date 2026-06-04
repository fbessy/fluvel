// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "temporal_mean.hpp"
#include "image_conversions.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace fluvel_ip::filter
{

// ------------------------------------------------------------
// RESET
// ------------------------------------------------------------
void TemporalMean::reset(const ImageView& first_src)
{
    assert(first_src.channels() >= 3);

    output_ = ImageOwner(first_src.width(), first_src.height(), ImageFormat::Bgr32);

    accumInit_ = false;
    noiseEstimInit_ = false;
    hasPreviousFrame_ = false;

    accum_.resize(first_src.width(), first_src.height());

    for (int y = 0; y < accum_.height(); ++y)
    {
        for (int x = 0; x < accum_.width(); ++x)
        {
            const Rgb_uc color = first_src.atPixelRgb(x, y);

            accum_.at(x, y) = Rgb_f{static_cast<float>(color.red), static_cast<float>(color.green),
                                    static_cast<float>(color.blue)};
        }
    }
    outputNeedsRefresh_ = true;

    const int pixelsCount = accum_.width() * accum_.height();
    constexpr int targetSamples = 200000;

    samplingStep_ = std::max(1, int(std::sqrt(float(pixelsCount) / targetSamples)));

    motionRatioFiltered_ = 1.f;

    accumInit_ = true;

    afterResetTimer_.start();
}

// ------------------------------------------------------------
// UPDATE
// ------------------------------------------------------------
void TemporalMean::update(const ImageView& src)
{
    assert(src.width() == accum_.width());
    assert(src.height() == accum_.height());

    if (!accumInit_)
        return;

    // --------------------------------------------------------
    // 1) Compute dt (FPS independent)
    // --------------------------------------------------------
    float deltaSeconds = 0.033f;

    if (hasPreviousFrame_)
        deltaSeconds = deltaFrameTimer_.elapsedSec<float>();

    deltaSeconds = std::clamp(deltaSeconds, 0.001f, 0.5f);

    // --------------------------------------------------------
    // 2) Compute motion (Root Mean Square color difference)
    // --------------------------------------------------------
    float motion = 0.f;
    int count = 0;

    for (int y = 0; y < accum_.height(); y += samplingStep_)
    {
        for (int x = 0; x < accum_.width(); x += samplingStep_)
        {
            const Rgb_uc s = src.atPixelRgb(x, y);
            const Rgb_f& a = accum_.at(x, y);

            float dr = float(s.red) - a.red;
            float dg = float(s.green) - a.green;
            float db = float(s.blue) - a.blue;

            motion += dr * dr + dg * dg + db * db;
            ++count;
        }
    }

    motion = std::sqrt(motion / (3.f * count));

    // --------------------------------------------------------
    // 3) Noise estimation (robust to spikes)
    // --------------------------------------------------------

    float motionForNoise = motion;

    if (noiseEstimInit_)
    {
        // Prevent large scene changes from instantly inflating
        // the estimated noise floor.
        const float maxAllowed = noiseEstimate_ * 3.f;
        motionForNoise = std::min(motionForNoise, maxAllowed);
    }

    updateNoiseEstimate(motionForNoise, deltaSeconds);

    // --------------------------------------------------------
    // 4) Motion normalized by noise
    // --------------------------------------------------------
    constexpr float epsilon = 1e-6f;

    float motionRatio = motion / (noiseEstimate_ + epsilon);

    // Petit filtre rapide sur le ratio
    const float tauRatio = 0.1f;
    float alphaRatio = 1.f - std::exp(-deltaSeconds / tauRatio);

    motionRatioFiltered_ += alphaRatio * (motionRatio - motionRatioFiltered_);

    // --------------------------------------------------------
    // 5) Linear mapping to tau
    // --------------------------------------------------------
    const float tauMin = 0.02f; // very reactive
    const float tauMax = 0.2f;  // stable

    const float ratioLow = 1.1f;
    const float ratioHigh = 1.6f;

    float t = 0.f;

    if (motionRatioFiltered_ > ratioLow)
    {
        t = (motionRatioFiltered_ - ratioLow) / (ratioHigh - ratioLow);
    }

    t = std::clamp(t, 0.f, 1.f);

    float tCurve = std::sqrt(t);

    float tau = tauMax - tCurve * (tauMax - tauMin);

    // --------------------------------------------------------
    // 6) Convert tau -> alpha (continuous-time correct)
    // --------------------------------------------------------
    float alpha = 1.f - std::exp(-deltaSeconds / tau);

    // --------------------------------------------------------
    // 7) Apply smoothing
    // --------------------------------------------------------

    const int channels = src.channels();

    for (int y = 0; y < accum_.height(); ++y)
    {
        const uint8_t* row = src.row(y);

        for (int x = 0; x < accum_.width(); ++x)
        {
            const int idx = x * channels;

            float r, g, b;
            src.readPixelRgb(row, idx, r, g, b);

            Rgb_f& a = accum_.at(x, y);

            a.red += alpha * (r - a.red);
            a.green += alpha * (g - a.green);
            a.blue += alpha * (b - a.blue);
        }
    }
    outputNeedsRefresh_ = true;

    hasPreviousFrame_ = true;
    deltaFrameTimer_.start();
}

// ------------------------------------------------------------
// Noise estimation (FPS independent)
// ------------------------------------------------------------
void TemporalMean::updateNoiseEstimate(float motion, float deltaSeconds)
{
    // Constantes de temps
    const float tauInit = 0.3f; // convergence rapide au démarrage
    const float tauUp = 0.3f;   // si le bruit augmente
    const float tauDown = 2.0f; // si le bruit diminue

    if (!noiseEstimInit_)
    {
        noiseEstimate_ = motion;
        noiseEstimInit_ = true;
        return;
    }

    // Choix de la constante selon la direction
    float tau = tauDown;

    if (motion > noiseEstimate_)
        tau = tauUp;

    // Faster convergence during the startup phase.
    if (afterResetTimer_.elapsedSec() < 0.5f)
        tau = tauInit;

    float alpha = 1.f - std::exp(-deltaSeconds / tau);

    noiseEstimate_ += alpha * (motion - noiseEstimate_);
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
