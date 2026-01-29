#include "temporal_smoother.hpp"
#include <algorithm>
#include <iostream>
namespace ofeli_ip
{

void TemporalSmoother::reset(ImageSpan first_src)
{
    initialized_ = false;

    accum_.resize(first_src.width(),
                  first_src.height() );

    for (int y = 0; y < accum_.height(); ++y)
    {
        for (int x = 0; x < accum_.width(); ++x)
        {
            const Rgb_uc s = first_src.atPixelRgb(x, y);

            accum_.at(x, y) = Rgb_f {
                static_cast<float>(s.red),
                static_cast<float>(s.green),
                static_cast<float>(s.blue)
            };
        }
    }

    initialized_ = true;
    noise_initialized_ = false;
}


void TemporalSmoother::update(ImageSpan src)
{
    if ( !initialized_ )
        return;

    float motion = 0.f;

    const float beta = 1.f - alpha_;

    for (int y = 0; y < accum_.height(); ++y)
    {
        for (int x = 0; x < accum_.width(); ++x)
        {
            const Rgb_uc  s = src.atPixelRgb(x, y);
            const Rgb_f&  a = accum_.at(x, y);

            Rgb_f out;
            out.red   = alpha_ * static_cast<float>(s.red)   + beta * a.red;
            out.green = alpha_ * static_cast<float>(s.green) + beta * a.green;
            out.blue  = alpha_ * static_cast<float>(s.blue)  + beta * a.blue;

            accum_.at(x, y) = out;

            motion +=   std::abs( static_cast<float>(s.red)   - a.red )
                      + std::abs( static_cast<float>(s.green) - a.green )
                      + std::abs( static_cast<float>(s.blue)  - a.blue );
        }
    }

    motion /= ( 3.f * accum_.size() );

    // adaptative smoothing
    // update alpha_ in function of the motion and noise

    updateNoiseEstimate( motion );
    float motion_eff = motion - noise_estimate_;
    motion_eff = std::max(0.f, motion_eff);
    const float motion_nl = motion_eff * motion_eff;

    const float threshold = 11.f;
    const float alpha_min = 0.05f;
    const float alpha_max = 0.75f;

    float t = std::clamp(motion_nl / threshold, 0.f, 1.f);
    t = std::sqrt(t);

    alpha_ = alpha_min + t * (alpha_max - alpha_min);
}

// called once per frame
void TemporalSmoother::updateNoiseEstimate(float motion)
{
    constexpr float noise_alpha = 0.02f;

    if ( !noise_initialized_ )
    {
        noise_estimate_ = motion;
        noise_initialized_ = true;
        return;
    }

    if ( motion < noise_estimate_ )
    {
        noise_estimate_ =
            (1.f - noise_alpha) * noise_estimate_
            + noise_alpha         * motion;
    }
}

void TemporalSmoother::updateOutput()
{
    output_.resize(accum_.width(), accum_.height());

    for (int y = 0; y < accum_.height(); ++y)
    {
        for (int x = 0; x < accum_.width(); ++x)
        {
            const Rgb_f& a = accum_.at(x, y);

            output_.at(x, y) = Rgb_uc {
                static_cast<unsigned char>( a.red   + 0.5f ),
                static_cast<unsigned char>( a.green + 0.5f ),
                static_cast<unsigned char>( a.blue  + 0.5f )
            };
        }
    }
}

ImageSpan TemporalSmoother::outputSpan()
{
    updateOutput();

    static_assert(sizeof(Rgb_uc)  == 3,  "Rgb_uc must be exactly 3 bytes");
    static_assert(alignof(Rgb_uc) == 1,  "Rgb_uc must be byte-aligned");
    static_assert(std::is_standard_layout_v<Rgb_uc>);

    return { reinterpret_cast<const unsigned char*>(output_.data()),
             output_.width(),
             output_.height(),
             ImageFormat::Rgb24 };
}

}
