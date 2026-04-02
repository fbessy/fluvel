// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "majority_internal_speed.hpp"

namespace fluvel_ip
{

MajorityInternalSpeed::MajorityInternalSpeed(int diskRadius, int gridWidth)
    : kernel_(makeInternalKernelOffsets(diskRadius, gridWidth))
{
}

InternalKernel MajorityInternalSpeed::makeInternalKernelOffsets(int diskRadius, int gridWidth)
{
    const int r = diskRadius;
    const int w = gridWidth;

    InternalKernel kernel;
    kernel.offsets.reserve(static_cast<std::size_t>((2 * r + 1) * (2 * r + 1)));

    kernel.support.min_dx = r;
    kernel.support.max_dx = -r;
    kernel.support.min_dy = r;
    kernel.support.max_dy = -r;

    for (int dy = -r; dy <= r; ++dy)
    {
        for (int dx = -r; dx <= r; ++dx)
        {
            if (dx * dx + dy * dy <= r * r)
            {
                kernel.offsets.push_back(dy * w + dx);

                kernel.support.min_dx = std::min(kernel.support.min_dx, dx);
                kernel.support.max_dx = std::max(kernel.support.max_dx, dx);
                kernel.support.min_dy = std::min(kernel.support.min_dy, dy);
                kernel.support.max_dy = std::max(kernel.support.max_dy, dy);
            }
        }
    }

    return kernel;
}

void MajorityInternalSpeed::computeSpeed(ContourPoint& point, const DiscreteLevelSet& phi)
{
    // Internal speed Fint is computed using a local majority vote
    // on the interior/exterior labeling of neighboring pixels.
    // This is equivalent to a smoothed Heaviside convolution
    // described in the reference paper.

    // Note: the direction is intentionally inverted.
    // If the neighborhood is mostly inside, the boundary locally
    // protrudes outward and must be pushed outward to smooth curvature.

    const int base = phi.offset(point.x(), point.y());

    int neighborOffset;

    int inside = 0;
    int outside = 0;

    if (kernel_.fully_inside(point.x(), point.y(), phi.width(), phi.height()))
    {
        // fast path without all neighbors existence checks

        for (int delta : kernel_.offsets)
        {
            neighborOffset = base + delta;

            PhiValue v = phi[neighborOffset];

            if (phi_value::isInside(v))
                ++inside;
            else
                ++outside;
        }
    }
    else
    {
        for (int delta : kernel_.offsets)
        {
            neighborOffset = base + delta;

            if (phi.valid(neighborOffset) /* all checks */)
            {
                PhiValue v = phi[neighborOffset];

                if (phi_value::isInside(v))
                    ++inside;
                else
                    ++outside;
            }
        }
    }

    // intentionally inverted, here.
    if (inside > outside)
        point.setSpeed(SpeedValue::GoOutward);
    else if (outside > inside)
        point.setSpeed(SpeedValue::GoInward);
    else
        point.setSpeed(SpeedValue::NoMove);
}

} // namespace fluvel_ip
