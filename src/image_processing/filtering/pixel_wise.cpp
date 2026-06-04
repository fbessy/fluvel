// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "pixel_wise.hpp"
#include "image_alpha.hpp"

#include <cassert>
#include <cstdint>

namespace fluvel_ip::pixelwise
{

void subtract(const ImageView& a, const ImageView& b, ImageOwner& out)
{
    assert(a.hasSameLayout(b));

    if (!a.hasSameLayout(b))
        return;

    out.ensureLike(a);

    const int w = out.width();
    const int h = out.height();

    const int channels = a.channels();
    const int activeChannels = std::min(channels, 3);

    for (int y = 0; y < h; ++y)
    {
        const uint8_t* pa = a.row(y);
        const uint8_t* pb = b.row(y);
        uint8_t* dst = out.rowPtr(y);

        for (int x = 0; x < w; ++x)
        {
            const int idx = x * channels;

            for (int c = 0; c < activeChannels; ++c)
            {
                const int diff = static_cast<int>(pa[idx + c]) - static_cast<int>(pb[idx + c]);
                dst[idx + c] = static_cast<uint8_t>(std::max(0, diff));
            }
        }
    }

    copyAlpha(a, out);
}

} // namespace fluvel_ip::pixelwise
