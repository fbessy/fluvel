// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "image_alpha.hpp"

#include <cassert>

namespace fluvel_ip
{

void copyAlpha(const ImageView& input, ImageOwner& output)
{
    assert(output.hasSameLayout(input));

    if (input.format() != ImageFormat::Bgr32 && input.format() != ImageFormat::Rgba32)
        return;

    const int w = input.width();
    const int h = input.height();

    for (int y = 0; y < h; ++y)
    {
        const uint8_t* src = input.row(y);
        uint8_t* dst = output.rowPtr(y);

        for (int x = 0; x < w; ++x)
        {
            dst[x * 4 + 3] = src[x * 4 + 3];
        }
    }
}

void fillAlpha(ImageOwner& image, uint8_t value)
{
    if (image.format() != ImageFormat::Bgr32 && image.format() != ImageFormat::Rgba32)
        return;

    const int width = image.width();
    const int height = image.height();

    for (int y = 0; y < height; ++y)
    {
        uint8_t* row = image.rowPtr(y);

        for (int x = 0; x < width; ++x)
        {
            row[x * 4 + 3] = value;
        }
    }
}

} // namespace fluvel_ip