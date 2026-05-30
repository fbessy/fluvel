// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "py_validation.hpp"

void validateImage(const py::array_t<uint8_t>& image)
{
    const auto ndim = image.ndim();

    if (ndim != 2 && ndim != 3)
        throw py::value_error("image must have shape (H,W), (H,W,3) or (H,W,4)");

    if (image.shape(0) < 1 || image.shape(1) < 1)
        throw py::value_error("image dimensions must be greater than zero");

    if (ndim == 3)
    {
        const auto channels = image.shape(2);

        if (channels != 3 && channels != 4)
            throw py::value_error("image must have 3 or 4 channels");
    }
}

void validateRadius(int radius)
{
    if (radius < 1)
        throw py::value_error("radius must be >= 1");
}

void validateSigma(float sigma)
{
    if (sigma <= 0.f)
        throw py::value_error("sigma must be greater than zero");
}

void validateProbability(float probability)
{
    if (probability < 0.f || probability > 1.f)
        throw py::value_error("probability must be in the range [0, 1]");
}