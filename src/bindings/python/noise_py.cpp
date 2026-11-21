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

seed
    Optional integer seed (0 to 2**32 - 1). With the same seed
    and the same image, the result is always identical.
    If omitted, the noise is different at each call.

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

seed
    Optional integer seed (0 to 2**32 - 1). With the same seed
    and the same image, the result is always identical.
    If omitted, the noise is different at each call.

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

seed
    Optional integer seed (0 to 2**32 - 1). With the same seed
    and the same image, the result is always identical.
    If omitted, the noise is different at each call.

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

seed
    Optional integer seed (0 to 2**32 - 1). With the same seed
    and the same image, the result is always identical.
    If omitted, the noise is different at each call.

Default
-------
sigma : 0.16

Returns
-------
numpy.ndarray

)";

// Seeds the random generator of the calling thread, for reproducible results.
void applySeed(const py::object& seed)
{
    if (seed.is_none())
        return;

    if (!py::isinstance<py::int_>(seed))
        throw py::type_error("seed must be an integer or None");

    const auto value = seed.cast<long long>();

    if (value < 0 || value > 0xFFFFFFFFLL)
        throw py::value_error("seed must be in the range [0, 2**32 - 1]");

    fluvel_ip::noise::rng().seed(static_cast<std::uint32_t>(value));
}

} // namespace

void bindNoise(py::module_& m)
{
    m.def(
        "gaussian_noise",

        [](const py::array& input, float sigma, const py::object& seed)
        {
            const auto array = toImageArray(input);
            validateSigma(sigma);
            applySeed(seed);

            auto img = pyArrayToImageView(array);

            auto out = runWithoutGil([&] { return fluvel_ip::noise::gaussian(img, sigma); });

            return imageOwnerToPyArray(out, input.ndim());
        },

        py::arg("image"), py::arg("sigma") = fluvel_ip::noise::kDefaultGaussianSigmaNoise,
        py::arg("seed") = py::none(), kGaussianNoiseDoc);

    m.def(
        "impulsive_noise",

        [](const py::array& input, float probability, const py::object& seed)
        {
            const auto array = toImageArray(input);
            validateProbability(probability);
            applySeed(seed);

            auto img = pyArrayToImageView(array);

            auto out = runWithoutGil([&] { return fluvel_ip::noise::impulsive(img, probability); });

            return imageOwnerToPyArray(out, input.ndim());
        },

        py::arg("image"), py::arg("probability") = fluvel_ip::noise::kDefaultImpulsiveNoise,
        py::arg("seed") = py::none(), kImpulsiveNoiseDoc);

    m.def(
        "speckle_uniform",

        [](const py::array& input, float sigma, const py::object& seed)
        {
            const auto array = toImageArray(input);
            validateSigma(sigma);
            applySeed(seed);

            auto img = pyArrayToImageView(array);

            auto out = runWithoutGil([&] { return fluvel_ip::noise::speckleUniform(img, sigma); });

            return imageOwnerToPyArray(out, input.ndim());
        },

        py::arg("image"), py::arg("sigma") = fluvel_ip::noise::kDefaultSpeckleNoise,
        py::arg("seed") = py::none(), kSpeckleUniformDoc);

    m.def(
        "speckle_gamma",

        [](const py::array& input, float sigma, const py::object& seed)
        {
            const auto array = toImageArray(input);
            validateSigma(sigma);
            applySeed(seed);

            auto img = pyArrayToImageView(array);

            auto out = runWithoutGil([&] { return fluvel_ip::noise::speckleGamma(img, sigma); });

            return imageOwnerToPyArray(out, input.ndim());
        },

        py::arg("image"), py::arg("sigma") = fluvel_ip::noise::kDefaultSpeckleNoise,
        py::arg("seed") = py::none(), kSpeckleGammaDoc);
}