// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ac_types.hpp"
#include "image_view.hpp"

namespace fluvel_ip
{

class ContourData;
class ContourDiagnostics;
enum class SwitchDirection;

class ISpeedModel
{
public:
    virtual ~ISpeedModel() = default;

    virtual void onImageChanged(ImageView image, const ContourData& contourData)
    {
    }

    virtual void onStepCycle1()
    {
    }

    //! Interface to compute the speed of one point.
    virtual void computeSpeed(ContourPoint& point, const DiscreteLevelSet& phi) = 0;

    virtual void onSwitch(const ContourPoint&, SwitchDirection)
    {
    }

    virtual void fillDiagnostics(ContourDiagnostics&) const
    {
    }
};

} // namespace fluvel_ip
