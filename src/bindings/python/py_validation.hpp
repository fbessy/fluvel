// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

/**
 * @brief Validate an image array received from Python.
 *
 * Supported image layouts are:
 * - Grayscale: (H, W)
 * - RGB:       (H, W, 3)
 * - RGBA:      (H, W, 4)
 *
 * Images must have non-zero dimensions.
 *
 * @param image NumPy image array to validate.
 *
 * @throws py::value_error If the image shape is invalid.
 */
void validateImage(const py::array_t<uint8_t>& image);

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