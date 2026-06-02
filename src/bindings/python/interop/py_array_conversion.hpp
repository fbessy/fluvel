// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <pybind11/numpy.h>

#include "image_owner.hpp"
#include "image_view.hpp"

namespace py = pybind11;

/**
 * @brief Convert a NumPy array into an ImageView.
 *
 * Creates a non-owning view over the memory contained in the provided
 * NumPy array. No pixel data is copied.
 *
 * The returned ImageView directly references the NumPy array buffer,
 * therefore the array must remain alive for the entire lifetime of
 * the ImageView.
 *
 * @param input Input NumPy array containing image data.
 * @return Non-owning image view referencing the NumPy array memory.
 */
fluvel_ip::ImageView pyArrayToImageView(const py::array_t<uint8_t>& input);

/**
 * @brief Convert an ImageOwner into a NumPy array.
 *
 * Allocates a new NumPy array and copies all image pixels into it.
 * The returned array owns its memory independently from the source image.
 *
 * @param image Source image.
 * @return NumPy array containing a copy of the image data.
 */
py::array_t<uint8_t> imageOwnerToPyArray(const fluvel_ip::ImageOwner& image);
