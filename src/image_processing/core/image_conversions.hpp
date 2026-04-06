// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"
#include "grid2d.hpp"
#include "image_owner.hpp"

namespace fluvel_ip
{

void convertRgbFToBgr32(const Grid2D<Rgb_f>& in, ImageOwner& out)
{
    assert(out.format() == ImageFormat::Bgr32);
    assert(out.width() == in.width());
    assert(out.height() == in.height());

    for (int y = 0; y < in.height(); ++y)
    {
        const Rgb_f* src = in.row(y);
        unsigned char* dst = out.data() + y * out.stride();

        for (int x = 0; x < in.width(); ++x)
        {
            const Rgb_f& p = src[x];

            dst[4 * x + 0] = static_cast<unsigned char>(std::clamp(p.blue + 0.5f, 0.f, 255.f));
            dst[4 * x + 1] = static_cast<unsigned char>(std::clamp(p.green + 0.5f, 0.f, 255.f));
            dst[4 * x + 2] = static_cast<unsigned char>(std::clamp(p.red + 0.5f, 0.f, 255.f));
            dst[4 * x + 3] = 255; // padding / alpha ignoré
        }
    }
}

} // namespace fluvel_ip
