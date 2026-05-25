// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <pybind11/numpy.h>

#include "contour_types.hpp"

namespace py = pybind11;

/**
 * @brief Convert a contour to a NumPy array.
 *
 * Output shape:
 *
 *     (N, 2)
 *
 * Columns:
 *
 *     [:,0] -> x
 *     [:,1] -> y
 *
 * @param contour Input contour.
 *
 * @return NumPy array of int32.
 */
py::array_t<int> contourToPyArray(const fluvel_ip::Contour& contour);