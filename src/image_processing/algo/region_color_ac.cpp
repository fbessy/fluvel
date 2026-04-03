// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "region_color_ac.hpp"
#include "contour_data.hpp"
#include "contour_diagnostics.hpp"
#include "fluvel_math.hpp"

#include <cassert>

namespace fluvel_ip
{

RegionColorSpeedModel::RegionColorSpeedModel(const RegionColorParams& params)
    : params_(params)
{
}

void RegionColorSpeedModel::onImageChanged(ImageView image, const ContourData& contour)
{
    assert(image.format() != ImageFormat::Gray8);
    assert(image.width() == contour.phi().width() && image.height() == contour.phi().height());

    initialMeansChecked_ = false;
    status_ = SpeedModelStatus::Ok;

    image_ = image;
    pixelCountTotal_ = image_.size();

    sumTotal_ = {0, 0, 0};

    sumOutside_ = {0, 0, 0};
    pixelCountOutside_ = 0;

    const auto& phi = contour.phi();

    for (int y = 0; y < phi.height(); ++y)
    {
        for (int x = 0; x < phi.width(); ++x)
        {
            const Rgb_64i rgb = static_cast<Rgb_64i>(image_.atPixelRgb(x, y));

            sumTotal_ += rgb;

            if (phi_value::isOutside(phi.at(x, y)))
            {
                sumOutside_ += rgb;
                ++pixelCountOutside_;
            }
        }
    }
}

void RegionColorSpeedModel::onStepCycle1()
{
    if (status_ != SpeedModelStatus::Ok)
        return;

    if (pixelCountOutside_ >= 1)
    {
        const Rgb<float> rgb = sumOutside_ / pixelCountOutside_;
        meanRgbOutside_ = rgb.rounded<unsigned char>();
        meanColorOut_ = rgbToColor(meanRgbOutside_);
    }
    else
    {
        status_ = SpeedModelStatus::RuntimeFailure;
    }

    const Rgb_64i sumIn = sumTotal_ - sumOutside_;
    const int64_t pxlNbrIn = pixelCountTotal_ - pixelCountOutside_;

    if (pxlNbrIn >= 1)
    {
        const Rgb<float> rgb = sumIn / pxlNbrIn;
        meanRgbInside_ = rgb.rounded<unsigned char>();
        meanColorIn_ = rgbToColor(meanRgbInside_);
    }
    else
    {
        status_ = SpeedModelStatus::RuntimeFailure;
    }

    if (!initialMeansChecked_)
    {
        if (status_ == SpeedModelStatus::Ok)
        {
            if (meanRgbOutside_ == meanRgbInside_)
                status_ = SpeedModelStatus::InitFailed;
        }

        initialMeansChecked_ = true;
    }
}

Color_3i RegionColorSpeedModel::rgbToColor(const Rgb_uc& rgb) const
{
    switch (params_.color_space)
    {
        case ColorSpaceOption::YUV:
            return color::rgb_to_yuv(rgb);

        case ColorSpaceOption::Lab:
        {
            const auto col = color::rgb_to_Lab(rgb);
            return scaleAndRound(col.L, col.a, col.b);
        }

        case ColorSpaceOption::Luv:
        {
            const auto col = color::rgb_to_Luv(rgb);
            return scaleAndRound(col.L, col.u, col.v);
        }

        case ColorSpaceOption::RGB:
            return {static_cast<int>(rgb.red), static_cast<int>(rgb.green),
                    static_cast<int>(rgb.blue)};
    }

    std::unreachable();
}

void RegionColorSpeedModel::computeSpeed(ContourPoint& point, const DiscreteLevelSet&)
{
    const Rgb_uc rgb = image_.atPixelRgb(point.x(), point.y());

    const auto col = rgbToColor(rgb);

    const int lambdaOut = params_.lambdaOut;
    const int lambdaIn = params_.lambdaIn;

    const auto weights = params_.weights;
    const auto meanColorOut = meanColorOut_;
    const auto meanColorIn = meanColorIn_;

    const Color_3i speedOut = weights * math::square(col - meanColorOut);
    const Color_3i speedIn = weights * math::square(col - meanColorIn);

    const int scalarSpeedOut = speedOut.scalar();
    const int scalarSpeedIn = speedIn.scalar();

    point.setSpeed(
        speed_value::get_discrete_speed(lambdaOut * scalarSpeedOut - lambdaIn * scalarSpeedIn));
}

void RegionColorSpeedModel::onSwitch(const ContourPoint& point, SwitchDirection direction)
{
    const Rgb_64i rgb = static_cast<Rgb_64i>(image_.atPixelRgb(point.x(), point.y()));

    if (direction == SwitchDirection::In)
    {
        sumOutside_ -= rgb;
        --pixelCountOutside_;
    }
    else if (direction == SwitchDirection::Out)
    {
        sumOutside_ += rgb;
        ++pixelCountOutside_;
    }
}

void RegionColorSpeedModel::fillDiagnostics(ContourDiagnostics& d) const
{
    d.meanInside =
        ChannelVector(static_cast<int>(meanRgbInside_.red), static_cast<int>(meanRgbInside_.green),
                      static_cast<int>(meanRgbInside_.blue));

    d.meanOutside = ChannelVector(static_cast<int>(meanRgbOutside_.red),
                                  static_cast<int>(meanRgbOutside_.green),
                                  static_cast<int>(meanRgbOutside_.blue));
}

} // namespace fluvel_ip
