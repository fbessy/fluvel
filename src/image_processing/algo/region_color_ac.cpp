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

#include "region_color_ac.hpp"
#include "ofeli_math.hpp"

#include <cassert>

namespace ofeli_ip
{

void RegionColorAc::initialize_sums()
{
    sum_total_ = {0, 0, 0};

    sum_out_ = {0, 0, 0};
    pxl_nbr_out_ = 0;

    for (int y = 0; y < cd_.phi().height(); ++y)
    {
        for (int x = 0; x < cd_.phi().width(); ++x)
        {
            const Rgb_64i rgb = static_cast<Rgb_64i>(image_.atPixelRgb(x, y));

            sum_total_ += rgb;

            if (phi_value::isOutside(cd_.phi().at(x, y)))
            {
                sum_out_ += rgb;
                ++pxl_nbr_out_;
            }
        }
    }
}

void RegionColorAc::do_specific_cycle1()
{
    if (pxl_nbr_out_ >= 1)
    {
        const auto rgb_f = sum_out_ / pxl_nbr_out_;
        average_out_ = rgb_f.rounded<unsigned char>();
        average_color_out_ = rgb_to_color(average_out_);
    }

    const Rgb_64i sum_in = sum_total_ - sum_out_;
    const int64_t pxl_nbr_in = pxl_nbr_total_ - pxl_nbr_out_;

    if (pxl_nbr_in >= 1)
    {
        const auto rgb_f = sum_in / pxl_nbr_in;
        average_in_ = rgb_f.rounded<unsigned char>();
        average_color_in_ = rgb_to_color(average_in_);
    }
}

Color_3i RegionColorAc::rgb_to_color(const Rgb_uc& rgb) const
{
    switch (region_config_.color_space)
    {
        case ColorSpaceOption::YUV:
            return color::rgb_to_yuv(rgb);

        case ColorSpaceOption::Lab:
        {
            const auto col = color::rgb_to_Lab(rgb);
            return scale_and_round(col.L, col.a, col.b);
        }

        case ColorSpaceOption::Luv:
        {
            const auto col = color::rgb_to_Luv(rgb);
            return scale_and_round(col.L, col.u, col.v);
        }

        case ColorSpaceOption::RGB:
            return {static_cast<int>(rgb.red), static_cast<int>(rgb.green),
                    static_cast<int>(rgb.blue)};
    }

    std::unreachable();
}

void RegionColorAc::compute_external_speed_Fd(ContourPoint& point)
{
    const Rgb_uc rgb = image_.atPixelRgb(point.x(), point.y());

    const auto col = rgb_to_color(rgb);

    const int lambda_out = region_config_.lambda_out;
    const int lambda_in = region_config_.lambda_in;

    const auto weights = region_config_.weights;
    const auto avg_out = average_color_out_;
    const auto avg_in = average_color_in_;

    const Color_3i veloc_out = weights * math::square(col - avg_out);
    const Color_3i veloc_in = weights * math::square(col - avg_in);

    const int speed_out = veloc_out.scalar();
    const int speed_in = veloc_in.scalar();

    point.speed_ = speed_value::get_discrete_speed(lambda_out * speed_out - lambda_in * speed_in);
}

void RegionColorAc::do_specific_when_switch(const ContourPoint& point, BoundarySwitch ctx_choice)
{
    const Rgb_64i rgb = static_cast<Rgb_64i>(image_.atPixelRgb(point.x(), point.y()));

    if (ctx_choice == BoundarySwitch::In)
    {
        sum_out_ -= rgb;
        --pxl_nbr_out_;
    }
    else if (ctx_choice == BoundarySwitch::Out)
    {
        sum_out_ += rgb;
        ++pxl_nbr_out_;
    }
}

void RegionColorAc::resetExecutionState(ImageSpan image)
{
    restart();

    image_ = image;

    initialize_sums();
    do_specific_cycle1();
}

} // namespace ofeli_ip
