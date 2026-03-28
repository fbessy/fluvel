// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

namespace fluvel_ip
{

class ContourData;
class ContourPoint;
class ContourDiagnostics;
enum class BoundarySwitch;

class ISpeedModel
{
public:
    virtual ~ISpeedModel() = default;

    virtual void initialize(const ContourData& cd)
    {
    }

    virtual void onStepCycle1()
    {
    } // default = no-op

    virtual void computeSpeed(ContourPoint& p) = 0;

    virtual void onSwitch(const ContourPoint&, BoundarySwitch)
    {
    } // no-op

    virtual void fillDiagnostics(ContourDiagnostics&) const
    {
    }
};

} // namespace fluvel_ip
