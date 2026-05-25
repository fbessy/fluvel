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

    const int h = out.height();
    const int rowBytes = out.rowBytes();

    for (int y = 0; y < h; ++y)
    {
        const uint8_t* pa = a.row(y);
        const uint8_t* pb = b.row(y);
        uint8_t* dst = out.rowPtr(y);

        for (int idx = 0; idx < rowBytes; ++idx)
        {
            int diff = int(pa[idx]) - int(pb[idx]);
            dst[idx] = static_cast<uint8_t>(std::max(0, diff));
        }
    }
}

} // namespace fluvel_ip::filter::pixelwise
