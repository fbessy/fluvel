// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "region_color_ac.hpp"
#include "fluvel_math.hpp"

#include <cassert>

namespace fluvel_ip
{

RegionColorAc::RegionColorAc(
    ImageView image, ContourData initialContour,
    const AcConfig& generalConfig,         /* optional parameter with AcConfig() */
    const RegionColorConfig& regionConfig) /* optional parameter with RegionColorConfig() */
    : ActiveContour(std::move(initialContour), generalConfig)
    , image_(image)
    , regionConfig_(regionConfig)
    , pxl_nbr_total_(image.size())
{
    assert(image.width() == cd_.phi().width() && image.height() == cd_.phi().height());

    initialize_sums();
    RegionColorAc::onStepCycle1();
}

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

void RegionColorAc::onStepCycle1()
{
    if (pxl_nbr_out_ >= 1)
    {
        const auto rgb_f = sum_out_ / pxl_nbr_out_;
        meanOut_ = rgb_f.rounded<unsigned char>();
        average_color_out_ = rgb_to_color(meanOut_);
    }

    const Rgb_64i sum_in = sum_total_ - sum_out_;
    const int64_t pxl_nbr_in = pxl_nbr_total_ - pxl_nbr_out_;

    if (pxl_nbr_in >= 1)
    {
        const auto rgb_f = sum_in / pxl_nbr_in;
        meanIn_ = rgb_f.rounded<unsigned char>();
        average_color_in_ = rgb_to_color(meanIn_);
    }
}

Color_3i RegionColorAc::rgb_to_color(const Rgb_uc& rgb) const
{
    switch (regionConfig_.color_space)
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

void RegionColorAc::computeSpeed(ContourPoint& point)
{
    const Rgb_uc rgb = image_.atPixelRgb(point.x(), point.y());

    const auto col = rgb_to_color(rgb);

    const int lambdaOut = regionConfig_.lambdaOut;
    const int lambdaIn = regionConfig_.lambdaIn;

    const auto weights = regionConfig_.weights;
    const auto avg_out = average_color_out_;
    const auto avg_in = average_color_in_;

    const Color_3i veloc_out = weights * math::square(col - avg_out);
    const Color_3i veloc_in = weights * math::square(col - avg_in);

    const int speed_out = veloc_out.scalar();
    const int speed_in = veloc_in.scalar();

    point.setSpeed(speed_value::get_discrete_speed(lambdaOut * speed_out - lambdaIn * speed_in));
}

void RegionColorAc::onSwitch(const ContourPoint& point, BoundarySwitch ctxChoice)
{
    const Rgb_64i rgb = static_cast<Rgb_64i>(image_.atPixelRgb(point.x(), point.y()));

    if (ctxChoice == BoundarySwitch::In)
    {
        sum_out_ -= rgb;
        --pxl_nbr_out_;
    }
    else if (ctxChoice == BoundarySwitch::Out)
    {
        sum_out_ += rgb;
        ++pxl_nbr_out_;
    }
}

void RegionColorAc::resetExecutionState(ImageView image)
{
    restart();

    image_ = image;

    initialize_sums();
    onStepCycle1();
}

void RegionColorAc::fillDiagnostics(ContourDiagnostics& d) const
{
    ActiveContour::fillDiagnostics(d);

    d.meanIn =
        ChannelVector(static_cast<int>(meanIn_.red), static_cast<int>(meanIn_.green),
                      static_cast<int>(meanIn_.blue));

    d.meanOut =
        ChannelVector(static_cast<int>(meanOut_.red), static_cast<int>(meanOut_.green),
                      static_cast<int>(meanOut_.blue));
}

} // namespace fluvel_ip
