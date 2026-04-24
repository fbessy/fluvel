// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"
#include "grid2d.hpp"
#include "image_owner.hpp"

namespace fluvel_ip
{

/**
 * @brief Convert a floating-point RGB grid to a BGR32 image.
 *
 * This function converts a Grid2D of floating-point RGB values (Rgb_f)
 * into an ImageOwner with Bgr32 format (8-bit per channel + alpha).
 *
 * Each channel is:
 * - rounded using +0.5f
 * - clamped to [0, 255]
 * - cast to uint8_t
 *
 * Output layout:
 * - B (byte 0)
 * - G (byte 1)
 * - R (byte 2)
 * - A (byte 3, set to 255)
 *
 * @param in Input grid of Rgb_f pixels.
 * @param out Output image (must be preallocated with matching size and Bgr32 format).
 *
 * @pre out.format() == ImageFormat::Bgr32
 * @pre out.width() == in.width()
 * @pre out.height() == in.height()
 *
 * @note Alpha channel is set to 255 (opaque) and not derived from input.
 */
void convertRgbFToBgr32(const Grid2D<Rgb_f>& in, ImageOwner& out)
{
    assert(out.format() == ImageFormat::Bgr32);
    assert(out.width() == in.width());
    assert(out.height() == in.height());

    for (int y = 0; y < in.height(); ++y)
    {
        const Rgb_f* src = in.row(y);
        uint8_t* dst = out.rowPtr(y);

        for (int x = 0; x < in.width(); ++x)
        {
            const Rgb_f& p = src[x];

            const int idx = 4 * x;

            dst[idx + 0] = static_cast<uint8_t>(std::clamp(p.blue + 0.5f, 0.f, 255.f));
            dst[idx + 1] = static_cast<uint8_t>(std::clamp(p.green + 0.5f, 0.f, 255.f));
            dst[idx + 2] = static_cast<uint8_t>(std::clamp(p.red + 0.5f, 0.f, 255.f));
            dst[idx + 3] = 255; // padding / alpha ignoré
        }
    }
}

} // namespace fluvel_ip
