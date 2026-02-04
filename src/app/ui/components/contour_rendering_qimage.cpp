/****************************************************************************
**
** Copyright (C) 2010-2025 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.
**
** http://www.cecill.info/licences/Licence_CeCILL_V2-en.html
** You may use this file under the terms of the CeCILL license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Fabien Bessy and its Subsidiary(-ies) nor the
**     names of its contributors may be used to endorse or promote products
**     derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

#include "contour_rendering_qimage.hpp"
#include "contour_data.hpp"
#include "common_settings.hpp"

#include <QImage>

namespace ofeli_app
{

void draw_list_to_img(const std::vector<ofeli_ip::ContourPoint>& list,
                      const ofeli_ip::Rgb_uc& color,
                      QImage& img)
{
    assert(!img.isNull());
    assert(img.format() == QImage::Format_RGB32);

    uchar* data = img.bits();
    const int stride = img.bytesPerLine();

    for (const auto& p : list)
    {
        assert(p.x() >= 0 && p.x() < img.width());
        assert(p.y() >= 0 && p.y() < img.height());

        uchar* px = data + p.y() * stride + 4 * p.x();

        // QImage::Format_RGB32: memory layout is B,G,R,0xFF
        px[0] = color.blue;
        px[1] = color.green;
        px[2] = color.red;
        px[3] = 255;
    }
}

void draw_upscale_list(const std::vector<ofeli_ip::ContourPoint>& list,
                       const ofeli_ip::Rgb_uc& color,
                       int upscale_factor,
                       QImage& img)
{
    if (upscale_factor != 2 && upscale_factor != 4)
        return;

    assert(!img.isNull());
    assert(img.format() == QImage::Format_RGB32);

    const int w = img.width();
    const int h = img.height();

    uchar* data = img.bits();
    const int stride = img.bytesPerLine();

    const int kernel_radius = upscale_factor / 2;

    for (const auto& point : list)
    {
        const int base_x = upscale_factor * point.x();
        const int base_y = upscale_factor * point.y();

        // fast path: whole kernel inside image
        if (   base_x + kernel_radius < w
            && base_y + kernel_radius < h
            && base_x - kernel_radius + 1 >= 0
            && base_y - kernel_radius + 1 >= 0 )
        {
            for (int dy = -kernel_radius + 1; dy <= kernel_radius; ++dy)
            {
                uchar* line = data + (base_y + dy) * stride;

                for (int dx = -kernel_radius + 1; dx <= kernel_radius; ++dx)
                {
                    uchar* px = line + 4 * (base_x + dx);

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

                uchar* line = data + y * stride;

                for (int dx = -kernel_radius + 1; dx <= kernel_radius; ++dx)
                {
                    const int x = base_x + dx;
                    if (x < 0 || x >= w)
                        continue;

                    uchar* px = line + 4 * x;

                    px[0] = color.blue;
                    px[1] = color.green;
                    px[2] = color.red;
                    px[3] = 255;
                }
            }
        }
    }
}

}
