// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "speed_model.hpp"

namespace fluvel_ip
{

//! Kernel support to know the geometry limit of the internal kernel.
struct KernelSupport
{
    int min_dx = 0;
    int max_dx = 0;
    int min_dy = 0;
    int max_dy = 0;
};

//! Precomputed disk-shaped kernel offsets for internal smoothing (Fint).
struct InternalKernel
{
    std::vector<int> offsets;
    KernelSupport support;

    bool fully_inside(int x, int y, int width, int height) const
    {
        return x + support.min_dx >= 0 && x + support.max_dx < width && y + support.min_dy >= 0 &&
               y + support.max_dy < height;
    }
};

class MajorityInternalSpeed : public ISpeedModel
{
public:
    MajorityInternalSpeed(int diskRadius, int gridWidth);

    //! Computes external speed \a Fd with the Chan-Vese model for a current point \a (x,y) of
    //! #outerBoundary or #innerBoundary.
    void computeSpeed(ContourPoint& point, const DiscreteLevelSet&) override;

private:
    //! Build kernel offsets.
    static InternalKernel makeInternalKernelOffsets(int diskRadius, int gridWidth);

    //! Precomputed disk-shaped kernel offsets for internal smoothing (Fint).
    const InternalKernel kernel_;
};

} // namespace fluvel_ip
