#ifndef TEMPORAL_SMOOTHER_HPP
#define TEMPORAL_SMOOTHER_HPP

#include "image_span.hpp"
#include "grid2d.hpp"
#include "color.hpp"

#include <chrono>

using clock_type = std::chrono::steady_clock;

namespace ofeli_ip
{

class TemporalSmoother
{
public:

    void reset(ImageSpan first_src);
    void update(ImageSpan src);

    const Grid2D<Rgb_f>& accum() const { return accum_; }

    ImageSpan outputSpan();

private:

    void updateNoiseEstimate(float motion,
                             float dt_seconds);
    void updateOutput();

    Grid2D<Rgb_f> accum_;
    bool initialized_ = false;

    int sampling_step_ = 1;
    float alpha_ = 0.1f;

    float noise_estimate_;
    bool noise_initialized_ = false;
    bool time_initialized_ = false;
    clock_type::time_point last_time_;
    float time_since_init_;

    bool high_motion_ = false;

    Grid2D<Rgb_uc> output_;

    float motion_ratio_filtered_ = 1.f;
};

}

#endif // TEMPORAL_SMOOTHER_HPP
