// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <pybind11/numpy.h>

#include "image_owner.hpp"
#include "image_view.hpp"

namespace py = pybind11;

fluvel_ip::ImageView pyArrayToImageView(const py::array_t<uint8_t>& input);
py::array_t<uint8_t> imageOwnerToPyArray(const fluvel_ip::ImageOwner& image);
