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

/**
 * @brief Naive morphological filter (dilation or erosion).
 *
 * This function applies a square structuring element of radius @p radius
 * using a straightforward implementation:
 *
 * - If IsMax = true  → dilation (max filter)
 * - If IsMax = false → erosion (min filter)
 *
 * The implementation iterates over all pixels in the neighborhood
 * (O(N * k²)), making it suitable for:
 * - small kernel sizes
 * - fallback when optimized algorithms (e.g. van Herk) are not applicable
 *
 * Border handling is performed using clamping (replication of edge pixels).
 *
 * @tparam IsMax Selects the operation:
 *         - true  → dilation (max)
 *         - false → erosion (min)
 *
 * @param input Input image view.
 * @param output Output image owner (must match input layout).
 * @param radius Radius of the structuring element (kernel size = 2 * radius + 1).
 *
 * @note Complexity is O(N * (2r+1)²).
 * @note This implementation is typically used as a reference or fallback.
 */
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
        uint8_t* dst = output.rowPtr(y);

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

                        int idx = xx * c + ch;
                        uint8_t val = row[idx];

                        if constexpr (IsMax)
                            v = std::max(v, val);
                        else
                            v = std::min(v, val);
                    }
                }

                int outIdx = x * c + ch;
                dst[outIdx] = v;
            }
        }
    }
}

} // namespace fluvel_ip::filter::morpho
