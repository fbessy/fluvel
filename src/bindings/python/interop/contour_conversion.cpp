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