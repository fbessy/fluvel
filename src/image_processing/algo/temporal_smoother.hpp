#ifndef TEMPORAL_SMOOTHER_HPP
#define TEMPORAL_SMOOTHER_HPP

#include "image_span.hpp"
#include "grid2d.hpp"
#include "color.hpp"

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

    void updateNoiseEstimate(float motion);
    void updateOutput();

    Grid2D<Rgb_f> accum_;
    bool initialized_ = false;

    float alpha_ = 0.1f;

    float noise_estimate_;
    bool noise_initialized_ = false;

    bool high_motion_ = false;

    Grid2D<Rgb_uc> output_;
};

}

#endif // TEMPORAL_SMOOTHER_HPP
