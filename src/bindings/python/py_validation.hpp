// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "active_contour_types.hpp"
#include "anisotropic_diffusion.hpp"
#include "speed_model_types.hpp"

namespace py = pybind11;

/// C-contiguous uint8 array, as expected by the image processing functions.
using PyImageArray = py::array_t<uint8_t, py::array::c_style>;

/**
 * @brief Validate an image array received from Python and make it contiguous.
 *
 * Only 8-bit images are accepted (dtype uint8). Supported layouts are:
 * - Grayscale: (H, W) or (H, W, 1)
 * - Color:     (H, W, 3), channels in BGR order
 * - Color+A:   (H, W, 4), channels in BGR order followed by an unused/alpha channel
 *
 * Images must have non-zero dimensions.
 *
 * Arrays that are not C-contiguous (slices with a step, transposed or
 * reversed views such as @c img[..., ::-1]) are copied.
 *
 * @param image NumPy array to validate.
 * @return C-contiguous uint8 array (the input itself when it already is).
 *
 * @throws py::type_error  If the dtype is not uint8.
 * @throws py::value_error If the shape is invalid.
 */
PyImageArray toImageArray(const py::array& image);

/**
 * @brief Validate a radius parameter received from Python.
 *
 * Radius values must be greater than or equal to 1.
 *
 * @param radius Radius value to validate.
 *
 * @throws pybind11::value_error If radius is invalid.
 */
void validateRadius(int radius);

/**
 * @brief Validate a Gaussian sigma parameter received from Python.
 *
 * The Gaussian standard deviation (sigma) must be strictly positive.
 *
 * Extremely small or large values are accepted and may be clamped
 * internally by the underlying implementation.
 *
 * @param sigma Gaussian standard deviation.
 *
 * @throws py::value_error If sigma is less than or equal to zero.
 */
void validateSigma(float sigma);

/**
 * @brief Validate a probability parameter received from Python.
 *
 * Probability values must lie within the inclusive range [0, 1].
 *
 * @param probability Probability value to validate.
 *
 * @throws py::value_error If probability is outside the range [0, 1].
 */
void validateProbability(float probability);

/**
 * @brief Validate the parameters of the anisotropic diffusion.
 *
 * Requires iterations >= 0, kappa > 0 and 0 < lambda <= 0.25
 * (larger time steps make the diffusion unstable).
 *
 * @throws py::value_error If a parameter is out of range.
 */
void validateAnisoParams(const fluvel_ip::filter::AnisoParams& params);

/**
 * @brief Validate the parameters of the active contour evolution.
 *
 * Requires Na >= 1, disk_radius >= 1 and, when the smoothing phase is
 * enabled, Ns >= 1.
 *
 * @throws py::value_error If a parameter is out of range.
 */
void validateActiveContourParams(const fluvel_ip::ActiveContourParams& params);

/**
 * @brief Validate the parameters of the region-based model.
 *
 * Requires lambda_in >= 0, lambda_out >= 0 and non-negative weights.
 *
 * @throws py::value_error If a parameter is out of range.
 */
void validateRegionParams(const fluvel_ip::RegionColorParams& params);
