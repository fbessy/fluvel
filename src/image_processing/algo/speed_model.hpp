// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ac_types.hpp"
#include "image_view.hpp"

namespace fluvel_ip
{

class ContourData;
struct ContourDiagnostics;
enum class SwitchDirection;

enum class SpeedModelStatus
{
    Ok,
    InitFailed,
    RuntimeFailure
};

class ISpeedModel
{
public:
    virtual ~ISpeedModel() = default;

    virtual void onImageChanged(ImageView, const ContourData&)
    {
    }

    virtual void onStepCycle1()
    {
    }

    //! Interface to compute the speed of one point.
    virtual void computeSpeed(ContourPoint&, const DiscreteLevelSet& /* phi */) = 0;

    virtual void onSwitch(const ContourPoint&, SwitchDirection)
    {
    }

    virtual void fillDiagnostics(ContourDiagnostics&) const
    {
    }

    virtual SpeedModelStatus status() const
    {
        return status_;
    }

protected:
    SpeedModelStatus status_{SpeedModelStatus::Ok};
};

} // namespace fluvel_ip
