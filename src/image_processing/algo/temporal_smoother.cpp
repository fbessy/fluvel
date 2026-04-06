// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "temporal_smoother.hpp"
#include "image_conversions.hpp"

#include <algorithm>
#include <cmath>

namespace fluvel_ip
{

// ------------------------------------------------------------
// RESET
// ------------------------------------------------------------
void TemporalSmoother::reset(ImageView first_src)
{
    output_ = ImageOwner(first_src.width(), first_src.height(), ImageFormat::Bgr32);

    initialized_ = false;
    noise_initialized_ = false;
    time_initialized_ = false;

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

    initialized_ = true;

    const int total_pixels = accum_.width() * accum_.height();
    constexpr int target_samples = 200000;

    sampling_step_ = std::max(1, int(std::sqrt(float(total_pixels) / target_samples)));
}

// ------------------------------------------------------------
// UPDATE
// ------------------------------------------------------------
void TemporalSmoother::update(ImageView src)
{
    if (!initialized_)
        return;

    // --------------------------------------------------------
    // 1) Compute dt (FPS independent)
    // --------------------------------------------------------
    float dt_seconds = 0.033f;

    if (!time_initialized_)
    {
        timer_.start();
        time_initialized_ = true;
    }
    else
    {
        dt_seconds = timer_.elapsedSec<float>();
        timer_.start();
    }

    dt_seconds = std::clamp(dt_seconds, 0.001f, 0.5f);

    // --------------------------------------------------------
    // 2) Compute motion (mean absolute difference)
    // --------------------------------------------------------
    float motion = 0.f;
    int count = 0;

    for (int y = 0; y < accum_.height(); y += sampling_step_)
    {
        for (int x = 0; x < accum_.width(); x += sampling_step_)
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

    float motion_for_noise = motion;

    // Clamp relatif : max 3x le bruit courant
    if (noise_initialized_)
    {
        float max_allowed = noise_estimate_ * 3.f;
        motion_for_noise = std::min(motion_for_noise, max_allowed);
    }

    updateNoiseEstimate(motion_for_noise, dt_seconds);

    // --------------------------------------------------------
    // 4) Motion normalized by noise
    // --------------------------------------------------------
    constexpr float epsilon = 1e-6f;

    float motion_ratio = motion / (noise_estimate_ + epsilon);

    // Petit filtre rapide sur le ratio
    const float tau_ratio = 0.1f;
    float alpha_ratio = 1.f - std::exp(-dt_seconds / tau_ratio);

    motion_ratio_filtered_ += alpha_ratio * (motion_ratio - motion_ratio_filtered_);

    // --------------------------------------------------------
    // 5) Linear mapping to tau
    // --------------------------------------------------------
    const float tau_min = 0.02f; // very reactive
    const float tau_max = 0.2f;  // stable

    const float ratio_low = 1.1f;
    const float ratio_high = 1.6f;

    float t = 0.f;

    if (motion_ratio_filtered_ > ratio_low)
    {
        t = (motion_ratio_filtered_ - ratio_low) / (ratio_high - ratio_low);
    }

    t = std::clamp(t, 0.f, 1.f);

    float t_curve = std::sqrt(t);

    float tau = tau_max - t_curve * (tau_max - tau_min);

    // --------------------------------------------------------
    // 6) Convert tau -> alpha (continuous-time correct)
    // --------------------------------------------------------
    float alpha = 1.f - std::exp(-dt_seconds / tau);

    // --------------------------------------------------------
    // 7) Apply smoothing
    // --------------------------------------------------------
    for (int y = 0; y < accum_.height(); ++y)
    {
        for (int x = 0; x < accum_.width(); ++x)
        {
            const Rgb_uc s = src.atPixelRgb(x, y);
            Rgb_f& a = accum_.at(x, y);

            a.red += alpha * (float(s.red) - a.red);
            a.green += alpha * (float(s.green) - a.green);
            a.blue += alpha * (float(s.blue) - a.blue);
        }
    }
}

// ------------------------------------------------------------
// Noise estimation (FPS independent)
// ------------------------------------------------------------
void TemporalSmoother::updateNoiseEstimate(float motion, float dt_seconds)
{
    // Constantes de temps
    const float tau_init = 0.3f; // convergence rapide au démarrage
    const float tau_up = 0.3f;   // si le bruit augmente
    const float tau_down = 2.0f; // si le bruit diminue

    if (!noise_initialized_)
    {
        noise_estimate_ = motion;
        noise_initialized_ = true;
        return;
    }

    // Choix de la constante selon la direction
    float tau = tau_down;

    if (motion > noise_estimate_)
        tau = tau_up;

    // Phase d'initialisation plus agressive pendant 0.5 s
    if (time_since_init_ < 0.5f)
        tau = tau_init;

    float alpha = 1.f - std::exp(-dt_seconds / tau);

    noise_estimate_ += alpha * (motion - noise_estimate_);

    time_since_init_ += dt_seconds;
}

// ------------------------------------------------------------
// OUTPUT
// ------------------------------------------------------------
void TemporalSmoother::updateOutput()
{
    convertRgbFToBgr32(accum_, output_);
}

ImageView TemporalSmoother::outputView()
{
    updateOutput();
    return output_.view();
}

const ImageOwner& TemporalSmoother::output()
{
    updateOutput();
    return output_;
}

} // namespace fluvel_ip
