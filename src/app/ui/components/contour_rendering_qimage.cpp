// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "contour_rendering_qimage.hpp"
#include "color.hpp"

#include <QImage>

namespace fluvel_app
{

void draw_list_to_img(const std::vector<fluvel_ip::ContourPoint>& list,
                      const fluvel_ip::Rgb_uc& color, QImage& img)
{
    assert(!img.isNull());
    assert(img.format() == QImage::Format_RGB32);

    uchar* data = img.bits();
    const qsizetype stride = img.bytesPerLine();

    for (const auto& p : list)
    {
        const qsizetype x = p.x();
        const qsizetype y = p.y();

        const qsizetype offset = y * stride + (x << 2);

        uchar* px = data + offset;

        px[0] = color.blue;
        px[1] = color.green;
        px[2] = color.red;
        px[3] = 255;
    }
}

void draw_upscale_list(const std::vector<fluvel_ip::ContourPoint>& list,
                       const fluvel_ip::Rgb_uc& color, int upscale_factor, QImage& img)
{
    if (upscale_factor != 2 && upscale_factor != 4)
        return;

    assert(!img.isNull());
    assert(img.format() == QImage::Format_RGB32);

    const int w = img.width();
    const int h = img.height();

    uchar* data = img.bits();
    const qsizetype stride = img.bytesPerLine();

    const int kernel_radius = upscale_factor / 2;

    for (const auto& point : list)
    {
        const int base_x = upscale_factor * point.x();
        const int base_y = upscale_factor * point.y();

        // fast path: whole kernel inside image
        if (base_x + kernel_radius < w && base_y + kernel_radius < h &&
            base_x - kernel_radius + 1 >= 0 && base_y - kernel_radius + 1 >= 0)
        {
            for (int dy = -kernel_radius + 1; dy <= kernel_radius; ++dy)
            {
                const qsizetype lineOffset = static_cast<qsizetype>(base_y + dy) * stride;

                uchar* line = data + lineOffset;

                for (int dx = -kernel_radius + 1; dx <= kernel_radius; ++dx)
                {
                    uchar* px = line + (static_cast<qsizetype>(base_x + dx) << 2);

                    px[0] = color.blue;
                    px[1] = color.green;
                    px[2] = color.red;
                    px[3] = 255;
                }
            }
        }
        else
        {
            // slow path with per-pixel bounds check
            for (int dy = -kernel_radius + 1; dy <= kernel_radius; ++dy)
            {
                const int y = base_y + dy;
                if (y < 0 || y >= h)
                    continue;

                const qsizetype lineOffset = static_cast<qsizetype>(y) * stride;

                uchar* line = data + lineOffset;

                for (int dx = -kernel_radius + 1; dx <= kernel_radius; ++dx)
                {
                    const int x = base_x + dx;
                    if (x < 0 || x >= w)
                        continue;

                    uchar* px = line + (static_cast<qsizetype>(x) << 2);

                    px[0] = color.blue;
                    px[1] = color.green;
                    px[2] = color.red;
                    px[3] = 255;
                }
            }
        }
    }
}

} // namespace fluvel_app
