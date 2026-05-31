// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "contour_conversion.hpp"

py::array_t<int> contourToPyArray(const fluvel_ip::Contour& contour)
{
    py::array_t<int> out({static_cast<ssize_t>(contour.size()), ssize_t(2)});

    auto view = out.mutable_unchecked<2>();

    for (size_t i = 0; i < contour.size(); ++i)
    {
        view(i, 0) = contour[i].x();

        view(i, 1) = contour[i].y();
    }

    return out;
}

fluvel_ip::Shape pyArrayToShape(const py::array_t<int>& contour)
{
    auto buffer = contour.request();

    if (buffer.ndim != 2)
        throw py::value_error("contour must be a 2D array of shape (N, 2)");

    if (buffer.shape[1] != 2)
        throw py::value_error("contour must have shape (N, 2)");

    const auto nbPoints = static_cast<size_t>(buffer.shape[0]);

    if (nbPoints == 0)
        throw py::value_error("contour must contain at least one point");

    auto points = contour.unchecked<2>();

    fluvel_ip::Shape shape;
    shape.reserve(nbPoints);

    for (size_t i = 0; i < nbPoints; ++i)
    {
        shape.pushBack(points(i, 0), points(i, 1));
    }

    shape.calculateCentroid();

    return shape;
}