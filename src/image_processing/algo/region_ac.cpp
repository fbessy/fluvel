// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "region_ac.hpp"

#include "ac_types.hpp"
#include "contour_data.hpp"
#include "contour_diagnostics.hpp"
#include "fluvel_math.hpp"

#include <cassert>
#include <cmath>

namespace fluvel_ip
{

RegionSpeedModel::RegionSpeedModel(
    const RegionParams& configuration) /* optional parameter with RegionParams() */
    : params_(configuration)
{
}

void RegionSpeedModel::onImageChanged(ImageView image, const ContourData& contour)
{
    assert(image.format() == ImageFormat::Gray8);
    assert(image.width() == contour.phi().width() && image.height() == contour.phi().height());

    initialMeansChecked_ = false;
    status_ = SpeedModelStatus::Ok;

    image_ = image;
    pxlNbrTotal_ = image_.size();

    sumTotal_ = 0;

    sumOut_ = 0;
    pxlNbrOut_ = 0;

    const auto& phi = contour.phi();

    for (int y = 0; y < phi.height(); ++y)
    {
        for (int x = 0; x < phi.width(); ++x)
        {
            const int64_t intensity = static_cast<int64_t>(image_.at(x, y));

            sumTotal_ += intensity;

            if (phi_value::isOutside(phi.at(x, y)))
            {
                sumOut_ += intensity;
                ++pxlNbrOut_;
            }
        }
    }
}

void RegionSpeedModel::onStepCycle1()
{
    if (status_ != SpeedModelStatus::Ok)
        return;

    if (pxlNbrOut_ >= 1)
        meanOut_ = std::lround(static_cast<float>(sumOut_) / pxlNbrOut_);
    else
        status_ = SpeedModelStatus::RuntimeFailure;

    const int64_t sumIn = sumTotal_ - sumOut_;
    const int64_t pxlNbrIn = pxlNbrTotal_ - pxlNbrOut_;

    if (pxlNbrIn >= 1)
        meanIn_ = std::lround(static_cast<float>(sumIn) / pxlNbrIn);
    else
        status_ = SpeedModelStatus::RuntimeFailure;

    if (!initialMeansChecked_)
    {
        if (status_ == SpeedModelStatus::Ok)
        {
            if (meanOut_ == meanIn_)
                status_ = SpeedModelStatus::InitFailed;

            initialMeansChecked_ = true;
        }
    }
}

void RegionSpeedModel::computeSpeed(ContourPoint& point, const DiscreteLevelSet&)
{
    const int pxl = static_cast<int>(image_.at(point.x(), point.y()));

    const int diffOut = pxl - meanOut_;
    const int diffIn = pxl - meanIn_;

    const int speed =
        params_.lambdaOut * (math::square(diffOut)) - params_.lambdaIn * (math::square(diffIn));

    point.setSpeed(speed_value::get_discrete_speed(speed));
}

void RegionSpeedModel::onSwitch(const ContourPoint& point, SwitchDirection direction)
{
    const int64_t intensity = static_cast<int64_t>(image_.at(point.x(), point.y()));

    if (direction == SwitchDirection::In)
    {
        sumOut_ -= intensity;
        --pxlNbrOut_;
    }
    else if (direction == SwitchDirection::Out)
    {
        sumOut_ += intensity;
        ++pxlNbrOut_;
    }
}

void RegionSpeedModel::fillDiagnostics(ContourDiagnostics& d) const
{
    d.meanIn = ChannelVector(meanIn_);
    d.meanOut = ChannelVector(meanOut_);
}

} // namespace fluvel_ip
