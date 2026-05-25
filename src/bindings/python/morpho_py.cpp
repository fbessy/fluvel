// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "morpho_py.hpp"

#include <iostream>
#include <pybind11/numpy.h>

#include "morpho.hpp"
#include "py_array_conversion.hpp"

namespace
{

constexpr auto kDilateDoc = R"(

Apply morphological dilation.

Computes the maximum value inside a square
structuring element.

Parameters
----------
image
    Input image.

radius
    Radius of the structuring element.

Returns
-------
numpy.ndarray
    Dilated image.

Examples
--------
>>> out = fluvel.dilate(
...     img,
...     radius=3
... )

)";

constexpr auto kErodeDoc = R"(

Apply morphological erosion.

Computes the minimum value inside a square
structuring element.

Parameters
----------
image
    Input image.

radius
    Radius of the structuring element.

Returns
-------
numpy.ndarray
    Eroded image.

Examples
--------
>>> out = fluvel.erode(
...     img,
...     radius=3
... )

)";

constexpr auto kOpeningDoc = R"(

Apply morphological opening.

Performs erosion followed by dilation.

Useful for removing small bright structures.

Parameters
----------
image
    Input image.

radius
    Radius of the structuring element.

Returns
-------
numpy.ndarray

)";

constexpr auto kClosingDoc = R"(

Apply morphological closing.

Performs dilation followed by erosion.

Useful for filling small dark regions.

Parameters
----------
image
    Input image.

radius
    Radius of the structuring element.

Returns
-------
numpy.ndarray

)";

constexpr auto kTopHatDoc = R"(

Apply white top-hat transform.

Computes:

    input - opening(input)

Highlights small bright structures.

Parameters
----------
image
    Input image.

radius
    Radius of the structuring element.

Returns
-------
numpy.ndarray

)";

constexpr auto kBlackTopHatDoc = R"(

Apply black top-hat transform.

Computes:

    closing(input) - input

Highlights small dark structures.

Parameters
----------
image
    Input image.

radius
    Radius of the structuring element.

Returns
-------
numpy.ndarray

)";

constexpr auto kGradientDoc = R"(

Apply morphological gradient.

Computes:

    dilation(input) - erosion(input)

Highlights transitions and edges.

Parameters
----------
image
    Input image.

radius
    Radius of the structuring element.

Returns
-------
numpy.ndarray

)";

} // namespace

void bindMorphology(py::module_& m)
{
    m.def(
        "dilate",

        [](py::array_t<uint8_t> input, int radius)
        {
            std::cout << "before pyArray\n";

            auto img = pyArrayToImageView(input);

            std::cout << img.width() << " " << img.height() << " " << img.channels() << " "
                      << img.stride() << "\n";

            std::cout << "before morpho\n";

            auto out = fluvel_ip::filter::morpho::max(img, radius);

            std::cout << "after morpho\n";

            return imageOwnerToPyArray(out);
        },

        py::arg("image"), py::arg("radius"), kDilateDoc);

    m.def(
        "erode",

        [](py::array_t<uint8_t> input, int radius)
        {
            auto img = pyArrayToImageView(input);

            auto out = fluvel_ip::filter::morpho::min(img, radius);

            return imageOwnerToPyArray(out);
        },

        py::arg("image"), py::arg("radius"), kErodeDoc);

    m.def(
        "opening",

        [](py::array_t<uint8_t> input, int radius)
        {
            auto img = pyArrayToImageView(input);

            auto out = fluvel_ip::filter::morpho::opening(img, radius);

            return imageOwnerToPyArray(out);
        },

        py::arg("image"), py::arg("radius"), kOpeningDoc);

    m.def(
        "closing",

        [](py::array_t<uint8_t> input, int radius)
        {
            auto img = pyArrayToImageView(input);

            auto out = fluvel_ip::filter::morpho::closing(img, radius);

            return imageOwnerToPyArray(out);
        },

        py::arg("image"), py::arg("radius"), kClosingDoc);

    m.def(
        "top_hat",

        [](py::array_t<uint8_t> input, int radius)
        {
            auto img = pyArrayToImageView(input);

            auto out = fluvel_ip::filter::morpho::topHat(img, radius);

            return imageOwnerToPyArray(out);
        },

        py::arg("image"), py::arg("radius"), kTopHatDoc);

    m.def(
        "black_top_hat",

        [](py::array_t<uint8_t> input, int radius)
        {
            auto img = pyArrayToImageView(input);

            auto out = fluvel_ip::filter::morpho::blackTopHat(img, radius);

            return imageOwnerToPyArray(out);
        },

        py::arg("image"), py::arg("radius"), kBlackTopHatDoc);

    m.def(
        "gradient",

        [](py::array_t<uint8_t> input, int radius)
        {
            auto img = pyArrayToImageView(input);

            auto out = fluvel_ip::filter::morpho::gradient(img, radius);

            return imageOwnerToPyArray(out);
        },

        py::arg("image"), py::arg("radius"), kGradientDoc);
}