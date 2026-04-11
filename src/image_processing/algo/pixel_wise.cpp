// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "pixel_wise.hpp"

#include <cassert>
#include <cstdint>

namespace fluvel_ip::filter::pixelwise
{

void diff(const ImageView& a, const ImageView& b, ImageOwner& out)
{
    assert(a.width() == b.width());
    assert(a.height() == b.height());
    assert(a.format() == b.format());

    assert(out.width() == a.width());
    assert(out.height() == a.height());
    assert(out.format() == a.format());

    const int w = a.width();
    const int h = a.height();
    const int c = a.channels();

    for (int y = 0; y < h; ++y)
    {
        const uint8_t* pa = a.row(y);
        const uint8_t* pb = b.row(y);
        uint8_t* dst = out.data() + y * out.stride();

        for (int i = 0; i < w * c; ++i)
        {
            int v = int(pa[i]) - int(pb[i]);
            dst[i] = static_cast<uint8_t>(std::max(0, v));
        }
    }
}

} // namespace fluvel_ip::filter::pixelwise
