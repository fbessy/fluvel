// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

namespace fluvel_ip::filter
{

void spatialFilter(const ImageView& input, ImageOwner& output);
ImageOwner spatialFilter(const ImageView& input);

class SpatialFilter
{
public:
    void reset(const ImageView& input);

    void apply(const ImageView& input);

    ImageView outputView() const;
    const ImageOwner& output() const;

private:
    void horizontalPass(const uint8_t* src, uint8_t* dst, int width, int channels);
    void verticalPass(const uint8_t* row_m1, const uint8_t* row_0, const uint8_t* row_p1,
                      uint8_t* dst, int width, int channels);

    ImageOwner buffer1_;
    ImageOwner buffer2_;

    int width_ = 0;
    int height_ = 0;
};

} // namespace fluvel_ip
