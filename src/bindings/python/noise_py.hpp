// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <pybind11/pybind11.h>

namespace py = pybind11;

/**
 * @brief Register noise-related Python bindings.
 *
 * Exposes synthetic noise generation utilities.
 */
void bindNoise(py::module_& m);