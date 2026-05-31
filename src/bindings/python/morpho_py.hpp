// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <pybind11/pybind11.h>

namespace py = pybind11;

/**
 * @brief Register the morphology API in a Python module.
 *
 * Adds morphology classes, functions and constants to the provided
 * Python module.
 *
 * @param m Target Python module.
 */
void bindMorphology(py::module_& m);