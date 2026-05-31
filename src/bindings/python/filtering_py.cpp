// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "filtering_py.hpp"

#include <pybind11/numpy.h>

#include "anisotropic_diffusion.hpp"
#include "mean_filter.hpp"
#include "median_filter.hpp"
#include "morpho_py.hpp"
#include "py_array_conversion.hpp"
#include "py_validation.hpp"

namespace
{

constexpr auto kConductionFunctionDoc = R"(

Conduction functions used by anisotropic diffusion.

The conduction function controls how image gradients
influence diffusion strength.

Attributes
----------
Exponential
    Exponential conduction function:

        exp(-(∇I / kappa)^2)

    Preserves strong edges more aggressively.

Rational
    Rational conduction function:

        1 / (1 + (∇I / kappa)^2)

    Produces a smoother transition between diffusion regions.

)";

constexpr auto kAnisoParamsDoc = R"(
Parameters controlling anisotropic diffusion.

Attributes
----------
conduction
    Conduction model.

iterations
    Number of diffusion iterations.

lambda_
    Time step.

kappa
    Edge threshold parameter.

Defaults
--------
iterations : 10
lambda_ : 1 / 7
kappa : 30
)";

constexpr auto kAnisoDiffusionDoc = R"(

Apply anisotropic diffusion.

Parameters
----------
image
    Input image.

params
    Optional diffusion parameters.

Returns
-------
numpy.ndarray
    Filtered image.

Examples
--------
>>> out = fluvel.anisotropic_diffusion(img)

>>> p = fluvel.AnisoParams()
>>> p.iterations = 20
>>> out = fluvel.anisotropic_diffusion(img, p)

)";

constexpr auto kMeanDoc = R"(

Apply a mean (box) filter.

Applies a mean filter with the specified radius.

The implementation may internally select an optimized
strategy depending on the radius.

Parameters
----------
image
    Input image.

radius
    Filter radius.

    Small radius values may use specialized
    optimized implementations.

Returns
-------
numpy.ndarray
    Filtered image.

Examples
--------
>>> out = fluvel.mean(
...     img,
...     radius=3
... )

)";

constexpr auto kMedianDoc = R"(

Apply a median filter.

Applies a median filter with the specified radius.

The implementation may internally select an optimized
strategy depending on the radius.

Optimized implementations may include histogram-based
approaches for large kernels.

Parameters
----------
image
    Input image.

radius
    Filter radius.

    Kernel size is:

        2 * radius + 1

Returns
-------
numpy.ndarray
    Filtered image.

Examples
--------
>>> out = fluvel.median(
...     img,
...     radius=3
... )

>>> out = fluvel.median(
...     img,
...     radius=10
... )

Notes
-----
For large kernels, optimized implementations may be
selected automatically.

)";

} // namespace

void bindFiltering(py::module_& m)
{
    py::enum_<fluvel_ip::filter::ConductionFunction>(m, "ConductionFunction",
                                                     kConductionFunctionDoc)
        .value("Exponential", fluvel_ip::filter::ConductionFunction::Exponential)
        .value("Rational", fluvel_ip::filter::ConductionFunction::Rational);

    py::class_<fluvel_ip::filter::AnisoParams>(m, "AnisoParams", kAnisoParamsDoc)
        .def(py::init<>())

        .def_readwrite("conduction", &fluvel_ip::filter::AnisoParams::conduction)
        .def_readwrite("iterations", &fluvel_ip::filter::AnisoParams::iterations)
        .def_readwrite("lambda_", &fluvel_ip::filter::AnisoParams::lambda)
        .def_readwrite("kappa", &fluvel_ip::filter::AnisoParams::kappa);

    m.def(
        "anisotropic_diffusion",

        [](py::array_t<uint8_t> input, const fluvel_ip::filter::AnisoParams& params)
        {
            validateImage(input);

            auto img = pyArrayToImageView(input);

            auto out = fluvel_ip::filter::anisotropicDiffusion(img, params);

            return imageOwnerToPyArray(out);
        },

        py::arg("image"), py::arg("params") = fluvel_ip::filter::AnisoParams{}, kAnisoDiffusionDoc);

    m.def(
        "mean",

        [](py::array_t<uint8_t> input, int radius)
        {
            validateImage(input);
            validateRadius(radius);

            auto img = pyArrayToImageView(input);

            auto out = fluvel_ip::filter::mean(img, radius);

            return imageOwnerToPyArray(out);
        },

        py::arg("image"), py::arg("radius"), kMeanDoc);

    m.def(
        "median",

        [](py::array_t<uint8_t> input, int radius)
        {
            validateImage(input);
            validateRadius(radius);

            auto img = pyArrayToImageView(input);

            auto out = fluvel_ip::filter::median(img, radius);

            return imageOwnerToPyArray(out);
        },

        py::arg("image"), py::arg("radius"), kMedianDoc);

    bindMorphology(m);
}