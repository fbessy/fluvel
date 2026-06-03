// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "noise_py.hpp"

#include <pybind11/numpy.h>

#include "noise.hpp"
#include "py_array_conversion.hpp"
#include "py_validation.hpp"

namespace py = pybind11;

namespace
{

constexpr auto kGaussianNoiseDoc = R"(

Apply additive Gaussian noise.

Adds zero-mean Gaussian noise.

Parameters
----------
image
    Input image.

sigma
    Standard deviation.

Default
-------
sigma : 20

Returns
-------
numpy.ndarray

Examples
--------
>>> out = fluvel.gaussian_noise(img)

>>> out = fluvel.gaussian_noise(
...     img,
...     sigma=30
... )

)";

constexpr auto kImpulsiveNoiseDoc = R"(

Apply salt-and-pepper noise.

Parameters
----------
image
    Input image.

probability
    Corruption probability.

Default
-------
probability : 0.05

Returns
-------
numpy.ndarray

Examples
--------
>>> out = fluvel.impulsive_noise(img)

>>> out = fluvel.impulsive_noise(
...     img,
...     probability=0.1
... )

)";

constexpr auto kSpeckleUniformDoc = R"(

Apply multiplicative speckle noise.

Uniform random distribution.

Parameters
----------
image
    Input image.

sigma
    Noise strength.

Default
-------
sigma : 0.16

Returns
-------
numpy.ndarray

)";

constexpr auto kSpeckleGammaDoc = R"(

Apply multiplicative speckle noise.

Gamma random distribution.

Parameters
----------
image
    Input image.

sigma
    Noise strength.

Default
-------
sigma : 0.16

Returns
-------
numpy.ndarray

)";

} // namespace

void bindNoise(py::module_& m)
{
    m.def(
        "gaussian_noise",

        [](const py::array_t<uint8_t>& input, float sigma)
        {
            validateImage(input);
            validateSigma(sigma);

            auto img = pyArrayToImageView(input);

            auto out = fluvel_ip::noise::gaussian(img, sigma);

            return imageOwnerToPyArray(out);
        },

        py::arg("image"), py::arg("sigma") = fluvel_ip::noise::kDefaultGaussianSigmaNoise,
        kGaussianNoiseDoc);

    m.def(
        "impulsive_noise",

        [](const py::array_t<uint8_t>& input, float probability)
        {
            validateImage(input);
            validateProbability(probability);

            auto img = pyArrayToImageView(input);

            auto out = fluvel_ip::noise::impulsive(img, probability);

            return imageOwnerToPyArray(out);
        },

        py::arg("image"), py::arg("probability") = fluvel_ip::noise::kDefaultImpulsiveNoise,
        kImpulsiveNoiseDoc);

    m.def(
        "speckle_uniform",

        [](const py::array_t<uint8_t>& input, float sigma)
        {
            validateImage(input);
            validateSigma(sigma);

            auto img = pyArrayToImageView(input);

            auto out = fluvel_ip::noise::speckleUniform(img, sigma);

            return imageOwnerToPyArray(out);
        },

        py::arg("image"), py::arg("sigma") = fluvel_ip::noise::kDefaultSpeckleNoise,
        kSpeckleUniformDoc);

    m.def(
        "speckle_gamma",

        [](const py::array_t<uint8_t>& input, float sigma)
        {
            validateImage(input);
            validateSigma(sigma);

            auto img = pyArrayToImageView(input);

            auto out = fluvel_ip::noise::speckleGamma(img, sigma);

            return imageOwnerToPyArray(out);
        },

        py::arg("image"), py::arg("sigma") = fluvel_ip::noise::kDefaultSpeckleNoise,
        kSpeckleGammaDoc);
}