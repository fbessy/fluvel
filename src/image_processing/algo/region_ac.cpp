// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "region_ac.hpp"

#include "fluvel_math.hpp"
#include <cmath>

namespace fluvel_ip
{

void RegionAc::initialize_sums()
{
    sum_total_ = 0;

    sum_out_ = 0;
    pxl_nbr_out_ = 0;

    for (int y = 0; y < cd_.phi().height(); ++y)
    {
        for (int x = 0; x < cd_.phi().width(); ++x)
        {
            const int64_t intensity = static_cast<int64_t>(image_.at(x, y));

            sum_total_ += intensity;

            if (phi_value::isOutside(cd_.phi().at(x, y)))
            {
                sum_out_ += intensity;
                ++pxl_nbr_out_;
            }
        }
    }
}

void RegionAc::do_specific_cycle1()
{
    if (pxl_nbr_out_ >= 1)
        meanOut_ = std::lround(static_cast<float>(sum_out_) / pxl_nbr_out_);

    const int64_t sum_in = sum_total_ - sum_out_;
    const int64_t pxl_nbr_in = pxl_nbr_total_ - pxl_nbr_out_;

    if (pxl_nbr_in >= 1)
        meanIn_ = std::lround(static_cast<float>(sum_in) / pxl_nbr_in);
}

void RegionAc::computeExternalSpeedFd(ContourPoint& point)
{
    const int pxl = static_cast<int>(image_.at(point.x(), point.y()));

    const int diffOut = pxl - meanOut_;
    const int diffIn = pxl - meanIn_;

    const int speed = regionConfig_.lambdaOut * (math::square(diffOut)) -
                      regionConfig_.lambdaIn * (math::square(diffIn));

    point.speed_ = speed_value::get_discrete_speed(speed);
}

void RegionAc::doSpecificWhenSwitch(const ContourPoint& point, BoundarySwitch ctxChoice)
{
    const int64_t intensity = static_cast<int64_t>(image_.at(point.x(), point.y()));

    if (ctxChoice == BoundarySwitch::In)
    {
        sum_out_ -= intensity;
        --pxl_nbr_out_;
    }
    else if (ctxChoice == BoundarySwitch::Out)
    {
        sum_out_ += intensity;
        ++pxl_nbr_out_;
    }
}

void RegionAc::fillDiagnostics(ContourDiagnostics& d) const
{
    ActiveContour::fillDiagnostics(d);

    d.meanIn = ChannelVector(meanIn_);
    d.meanOut = ChannelVector(meanOut_);
}

} // namespace fluvel_ip
