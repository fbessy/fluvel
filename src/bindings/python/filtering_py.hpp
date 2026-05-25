// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <pybind11/pybind11.h>

namespace py = pybind11;

/**
 * @brief Register filtering-related Python bindings.
 *
 * Exposes image filtering utilities and parameter objects.
 *
 * @param m Python module receiving the filtering bindings.
 */
void bindFiltering(py::module_& m);
