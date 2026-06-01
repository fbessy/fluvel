// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <pybind11/numpy.h>

#include "contour_types.hpp"
#include "shape.hpp"

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
py::array_t<int> contourToPyArray(const fluvel_ip::ContourPoints& contour);

/**
 * @brief Convert a NumPy contour array to a Shape.
 *
 * The input contour must be a 2D NumPy array of shape (N, 2),
 * where each row contains the coordinates of a contour point:
 *
 * @code
 * [[x0, y0],
 *  [x1, y1],
 *  ...]
 * @endcode
 *
 * Coordinates follow the same convention as Point2D_i:
 *
 * @code
 * contour[i, 0] -> x
 * contour[i, 1] -> y
 * @endcode
 *
 * @note Coordinates are expressed in geometric order (x, y),
 *       not image indexing order (row, column).
 *
 * The resulting Shape contains all contour points and its centroid
 * is computed automatically.
 *
 * @param contour Input contour as a NumPy array of shape (N, 2).
 *
 * @return Shape built from the contour points.
 *
 * @throw py::value_error If:
 *         - the array is not two-dimensional,
 *         - the array shape is not (N, 2),
 *         - the contour contains no points.
 */
fluvel_ip::Shape pyArrayToShape(const py::array_t<int>& contour);