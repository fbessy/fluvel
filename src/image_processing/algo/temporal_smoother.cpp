#include "temporal_smoother.hpp"

#include <algorithm>
#include <cmath>
#include <chrono>

namespace ofeli_ip
{

using clock_type = std::chrono::steady_clock;

// ------------------------------------------------------------
// RESET
// ------------------------------------------------------------
void TemporalSmoother::reset(ImageSpan first_src)
{
    initialized_ = false;
    noise_initialized_ = false;
    time_initialized_ = false;

    accum_.resize(first_src.width(),
                  first_src.height());

    for (int y = 0; y < accum_.height(); ++y)
    {
        for (int x = 0; x < accum_.width(); ++x)
        {
            const Rgb_uc color = first_src.atPixelRgb(x, y);

            accum_.at(x, y) = Rgb_f {
                static_cast<float>(color.red),
                static_cast<float>(color.green),
                static_cast<float>(color.blue)
            };
        }
    }

    initialized_ = true;
}

// ------------------------------------------------------------
// UPDATE
// ------------------------------------------------------------
void TemporalSmoother::update(ImageSpan src)
{
    if (!initialized_)
        return;

    // --------------------------------------------------------
    // 1) Compute dt (FPS independent)
    // --------------------------------------------------------
    float dt_seconds = 0.033f;

    const auto now = clock_type::now();

    if (!time_initialized_)
    {
        last_time_ = now;
        time_initialized_ = true;
    }
    else
    {
        std::chrono::duration<float> dt = now - last_time_;
        dt_seconds = dt.count();
        last_time_ = now;
    }

    dt_seconds = std::clamp(dt_seconds, 0.001f, 0.5f);

    // --------------------------------------------------------
    // 2) Compute motion (mean absolute difference)
    // --------------------------------------------------------
    float motion = 0.f;

    for (int y = 0; y < accum_.height(); ++y)
    {
        for (int x = 0; x < accum_.width(); ++x)
        {
            const Rgb_uc  s = src.atPixelRgb(x, y);
            const Rgb_f&  a = accum_.at(x, y);

            motion += std::abs(float(s.red)   - a.red)
                      + std::abs(float(s.green) - a.green)
                      + std::abs(float(s.blue)  - a.blue);
        }
    }

    motion /= (3.f * accum_.size());

    // --------------------------------------------------------
    // 3) Noise estimation (time-based)
    // --------------------------------------------------------
    updateNoiseEstimate(motion, dt_seconds);

    // --------------------------------------------------------
    // 4) Motion normalized by noise
    // --------------------------------------------------------
    constexpr float epsilon = 1e-6f;

    float motion_ratio = motion / (noise_estimate_ + epsilon);

    // ratio <= 1 → noise only
    float ratio_eff = std::max(0.f, motion_ratio - 1.f);

    // --------------------------------------------------------
    // 5) Linear mapping to tau
    // --------------------------------------------------------
    const float tau_min = 0.1f;  // very reactive
    const float tau_max = 1.0f;  // stable

    const float ratio_threshold = 1.4f;
    // try between 1.4 and 1.8 for tuning

    float t = std::clamp(ratio_eff / ratio_threshold,
                         0.f, 1.f);

    float tau = tau_max - t * (tau_max - tau_min);

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

            a.red   += alpha * (float(s.red)   - a.red);
            a.green += alpha * (float(s.green) - a.green);
            a.blue  += alpha * (float(s.blue)  - a.blue);
        }
    }
}

// ------------------------------------------------------------
// Noise estimation (FPS independent)
// ------------------------------------------------------------
void TemporalSmoother::updateNoiseEstimate(float motion,
                                           float dt_seconds)
{
    const float tau_noise = 1.0f; // converge in ~3 seconds at 95%

    float alpha_noise = 1.f - std::exp(-dt_seconds / tau_noise);

    if (!noise_initialized_)
    {
        noise_estimate_ = motion;
        noise_initialized_ = true;
        return;
    }

    // Only track low-motion regions as noise
    if (motion < noise_estimate_)
    {
        noise_estimate_ +=
            alpha_noise * (motion - noise_estimate_);
    }
}

// ------------------------------------------------------------
// OUTPUT
// ------------------------------------------------------------
void TemporalSmoother::updateOutput()
{
    output_.resize(accum_.width(), accum_.height());

    for (int y = 0; y < accum_.height(); ++y)
    {
        for (int x = 0; x < accum_.width(); ++x)
        {
            const Rgb_f& a = accum_.at(x, y);

            output_.at(x, y) = Rgb_uc {
                static_cast<unsigned char>(a.red   + 0.5f),
                static_cast<unsigned char>(a.green + 0.5f),
                static_cast<unsigned char>(a.blue  + 0.5f)
            };
        }
    }
}

ImageSpan TemporalSmoother::outputSpan()
{
    updateOutput();

    static_assert(sizeof(Rgb_uc)  == 3);
    static_assert(alignof(Rgb_uc) == 1);
    static_assert(std::is_standard_layout_v<Rgb_uc>);

    return {
        reinterpret_cast<const unsigned char*>(output_.data()),
        output_.width(),
        output_.height(),
        ImageFormat::Rgb24
    };
}

} // namespace ofeli_ip
