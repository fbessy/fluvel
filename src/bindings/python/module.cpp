// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "active_contour_py.hpp"
#include "filtering_py.hpp"
#include "hausdorff_py.hpp"
#include "noise_py.hpp"

PYBIND11_MODULE(fluvel, m)
{
    m.doc() = "Fluvel Python bindings";

    bindNoise(m);
    bindFiltering(m);
    bindActiveContour(m);
    bindHausdorff(m);
}