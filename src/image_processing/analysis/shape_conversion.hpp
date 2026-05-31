// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "contour_types.hpp"
#include "shape.hpp"

namespace fluvel_ip
{

/**
 * @brief Convert a contour boundary to a geometric shape.
 *
 * The contour points are converted to Point2D_i coordinates and stored
 * in a Shape object suitable for geometric analysis algorithms such as
 * Hausdorff distance computation.
 *
 * The centroid of the resulting shape is computed automatically.
 *
 * @param contour Input contour boundary.
 *
 * @return Shape built from the contour points.
 */
Shape contourToShape(const Contour& contour);

/**
 * @brief Fill a Shape from a contour.
 *
 * Existing points stored in the shape are discarded.
 * The underlying storage is reused whenever possible to
 * minimize memory allocations.
 *
 * The centroid of the resulting shape is computed automatically.
 *
 * @param contour Input contour.
 * @param shape Output shape.
 */
void contourToShape(const Contour& contour, Shape& shape);

} // namespace fluvel_ip