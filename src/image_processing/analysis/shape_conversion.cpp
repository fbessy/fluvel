// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "shape_conversion.hpp"

namespace fluvel_ip
{

Shape contourToShape(const Contour& contour)
{
    Shape shape;
    shape.reserve(contour.size());

    for (const auto& point : contour)
        shape.pushBack(toPoint2D(point));

    shape.calculateCentroid();

    return shape;
}

void contourToShape(const Contour& contour, Shape& shape)
{
    shape.clear();

    shape.reserve(contour.size());

    for (const auto& point : contour)
        shape.pushBack(toPoint2D(point));

    shape.calculateCentroid();
}

} // namespace fluvel_ip