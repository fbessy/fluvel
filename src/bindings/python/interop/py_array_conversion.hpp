// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <pybind11/numpy.h>

#include "image_owner.hpp"
#include "image_view.hpp"

namespace py = pybind11;

/**
 * @brief Convert a C-contiguous NumPy array into an ImageView.
 *
 * Creates a non-owning view over the memory contained in the provided
 * NumPy array. No pixel data is copied.
 *
 * The returned ImageView directly references the NumPy array buffer,
 * therefore the array must remain alive for the entire lifetime of
 * the ImageView.
 *
 * @param input C-contiguous uint8 array (see toImageArray()).
 * @return Non-owning image view referencing the NumPy array memory.
 */
fluvel_ip::ImageView pyArrayToImageView(const py::array_t<uint8_t, py::array::c_style>& input);

/**
 * @brief Convert an ImageOwner into a NumPy array.
 *
 * Allocates a new NumPy array and copies all image pixels into it.
 * The returned array owns its memory independently from the source image.
 *
 * The result has the same number of dimensions as the input image:
 * a grayscale image given as (H, W) is returned as (H, W).
 *
 * @param image Source image.
 * @param inputNdim Number of dimensions of the input array (2 or 3).
 * @return NumPy array containing a copy of the image data.
 */
py::array_t<uint8_t> imageOwnerToPyArray(const fluvel_ip::ImageOwner& image, py::ssize_t inputNdim);

/**
 * @brief Run a computation with the Python GIL released.
 *
 * Must not touch any Python object. Other Python threads can run
 * while the computation is in progress.
 *
 * @param function Callable returning a C++ value.
 * @return Value returned by the callable.
 */
template <class Function>
decltype(auto) runWithoutGil(Function&& function)
{
    py::gil_scoped_release release;

    return function();
}
