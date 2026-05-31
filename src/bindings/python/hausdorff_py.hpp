// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <pybind11/pybind11.h>

namespace py = pybind11;

/**
 * @brief Register Hausdorff distance Python bindings.
 * @param m Python module to populate.
 */
void bindHausdorff(py::module_& m);