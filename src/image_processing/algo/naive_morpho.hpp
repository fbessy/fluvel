// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>

namespace fluvel_ip::filter::morpho
{

template <bool IsMax>
void naiveMorpho(const ImageView& input, ImageOwner& output, int radius)
{
    assert(output.width() == input.width());
    assert(output.height() == input.height());
    assert(output.format() == input.format());

    const int w = input.width();
    const int h = input.height();
    const int c = input.channels();

    for (int y = 0; y < h; ++y)
    {
        uint8_t* dst = output.data() + y * output.stride();

        for (int x = 0; x < w; ++x)
        {
            for (int ch = 0; ch < c; ++ch)
            {
                uint8_t v = IsMax ? 0 : 255;

                for (int ky = -radius; ky <= radius; ++ky)
                {
                    int yy = std::clamp(y + ky, 0, h - 1);
                    const uint8_t* row = input.row(yy);

                    for (int kx = -radius; kx <= radius; ++kx)
                    {
                        int xx = std::clamp(x + kx, 0, w - 1);
                        uint8_t val = row[xx * c + ch];

                        if constexpr (IsMax)
                            v = std::max(v, val);
                        else
                            v = std::min(v, val);
                    }
                }

                dst[x * c + ch] = v;
            }
        }
    }
}

} // namespace fluvel_ip::filter::morpho
