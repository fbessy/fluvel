// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"
#include "grid2d.hpp"
#include "image_owner.hpp"
#include "image_view.hpp"

#include "elapsed_timer.hpp"

namespace fluvel_ip
{

class TemporalSmoother
{
public:
    void reset(ImageView first_src);
    void update(ImageView src);

    const Grid2D<Rgb_f>& accum() const
    {
        return accum_;
    }

    ImageView output();

private:
    void updateNoiseEstimate(float motion, float dt_seconds);
    void updateOutput();

    Grid2D<Rgb_f> accum_;
    bool initialized_ = false;

    int sampling_step_ = 1;
    float alpha_ = 0.1f;

    float noise_estimate_;
    bool noise_initialized_ = false;
    bool time_initialized_ = false;
    ElapsedTimer timer_;
    float time_since_init_;

    bool high_motion_ = false;

    ImageOwner output_;

    float motion_ratio_filtered_ = 1.f;
};

} // namespace fluvel_ip
