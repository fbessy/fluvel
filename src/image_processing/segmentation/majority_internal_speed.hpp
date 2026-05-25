// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "speed_model.hpp"

namespace fluvel_ip
{

/**
 * @brief Defines the spatial extent of a kernel.
 *
 * This structure describes the bounding box of a kernel in terms of
 * relative offsets along x and y axes.
 *
 * It is mainly used to quickly check whether a kernel centered at a given
 * position lies entirely inside an image domain.
 */
struct KernelSupport
{
    int min_dx = 0; ///< Minimum horizontal offset (left extent).
    int max_dx = 0; ///< Maximum horizontal offset (right extent).
    int min_dy = 0; ///< Minimum vertical offset (top extent).
    int max_dy = 0; ///< Maximum vertical offset (bottom extent).
};

/**
 * @brief Precomputed disk-shaped kernel for internal smoothing.
 *
 * This structure stores the offsets of a disk-shaped kernel used for
 * internal speed computation (Fint).
 *
 * Offsets are stored in a linearized form (typically for fast iteration),
 * and the bounding box is provided via KernelSupport.
 */
struct InternalKernel
{
    std::vector<int> offsets; ///< Linearized offsets of the kernel.
    KernelSupport support;    ///< Bounding box of the kernel.

    /**
     * @brief Check if the kernel is fully inside the image domain.
     *
     * This allows avoiding boundary checks when applying the kernel.
     *
     * @param x Center x-coordinate.
     * @param y Center y-coordinate.
     * @param width Image width.
     * @param height Image height.
     * @return true if the kernel is entirely inside the image.
     */
    bool fully_inside(int x, int y, int width, int height) const
    {
        return x + support.min_dx >= 0 && x + support.max_dx < width && y + support.min_dy >= 0 &&
               y + support.max_dy < height;
    }
};

/**
 * @brief Internal speed model based on majority voting.
 *
 * This class implements an internal smoothing speed (Fint) using
 * a disk-shaped neighborhood. The speed is determined by the majority
 * of neighboring points (inside vs outside region).
 *
 * It is typically used in Cycle2 of the active contour algorithm to
 * regularize the contour and enforce smoothness.
 */
class MajorityInternalSpeed : public ISpeedModel
{
public:
    /**
     * @brief Construct the internal speed model.
     *
     * @param diskRadius Radius of the disk-shaped kernel.
     * @param gridWidth Width of the level-set grid (used for offset computation).
     */
    MajorityInternalSpeed(int diskRadius, int gridWidth);

    /**
     * @brief Compute internal speed for a contour point.
     *
     * This method evaluates the majority of neighboring pixels within
     * the disk kernel and assigns a speed accordingly.
     *
     * @param point Contour point to update.
     * @param phi Discrete level-set function.
     */
    void computeSpeed(ContourPoint& point, const DiscreteLevelSet&) override;

private:
    /**
     * @brief Build disk-shaped kernel offsets.
     *
     * Precomputes all offsets corresponding to a disk of given radius.
     *
     * @param diskRadius Radius of the kernel.
     * @param gridWidth Width of the grid (used for linear indexing).
     * @return Precomputed InternalKernel.
     */
    static InternalKernel makeInternalKernelOffsets(int diskRadius, int gridWidth);

    const InternalKernel kernel_; ///< Precomputed disk kernel.
};

} // namespace fluvel_ip
