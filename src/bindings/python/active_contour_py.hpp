// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <pybind11/pybind11.h>

namespace py = pybind11;

/**
 * @brief Register active contour Python bindings.
 * @param m Python module to populate.
 */
void bindActiveContour(py::module_& m);