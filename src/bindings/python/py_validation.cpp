// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "py_validation.hpp"

#include <string>

PyImageArray toImageArray(const py::array& image)
{
    // No silent cast: a float or 16-bit image would otherwise be truncated.
    if (!py::isinstance<py::array_t<uint8_t>>(image))
    {
        throw py::type_error("image must have dtype uint8, got " +
                             std::string(py::str(image.dtype())) +
                             ". Convert it first, for example with img.astype(np.uint8).");
    }

    const auto ndim = image.ndim();

    if (ndim != 2 && ndim != 3)
        throw py::value_error("image must have shape (H,W), (H,W,1), (H,W,3) or (H,W,4)");

    if (image.shape(0) < 1 || image.shape(1) < 1)
        throw py::value_error("image dimensions must be greater than zero");

    if (ndim == 3)
    {
        const auto channels = image.shape(2);

        if (channels != 1 && channels != 3 && channels != 4)
            throw py::value_error("image must have 1, 3 or 4 channels");
    }

    // Copies the data only if the array is not C-contiguous.
    auto contiguous = PyImageArray::ensure(image);

    if (!contiguous)
        throw py::value_error("image cannot be converted to a contiguous uint8 array");

    return contiguous;
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

void validateAnisoParams(const fluvel_ip::filter::AnisoParams& params)
{
    if (params.iterations < 0)
        throw py::value_error("iterations must be >= 0");

    if (!(params.kappa > 0.0))
        throw py::value_error("kappa must be greater than zero");

    // Above 1/4 the explicit scheme becomes unstable.
    if (!(params.lambda > 0.0 && params.lambda <= 0.25))
        throw py::value_error("lambda_ must be in the range ]0, 0.25]");
}

void validateActiveContourParams(const fluvel_ip::ActiveContourParams& params)
{
    // Same contract as the ActiveContour constructor.
    if (params.Na < 1)
        throw py::value_error("Na must be >= 1");

    if (params.cycle2Enabled && params.Ns < 1)
        throw py::value_error("Ns must be >= 1 when cycle2_enabled is True");

    if (params.diskRadius < 1)
        throw py::value_error("disk_radius must be >= 1");
}

void validateRegionParams(const fluvel_ip::RegionColorParams& params)
{
    if (params.lambdaIn < 0 || params.lambdaOut < 0)
        throw py::value_error("lambda_in and lambda_out must be >= 0");

    if (params.weights.c1 < 0 || params.weights.c2 < 0 || params.weights.c3 < 0)
        throw py::value_error("weights must be >= 0");
}
