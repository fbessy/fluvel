// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "active_contour_py.hpp"

#include <pybind11/numpy.h>

#include "active_contour.hpp"
#include "contour_data.hpp"
#include "contour_types.hpp"
#include "region_color_speed_model.hpp"
#include "region_gray_speed_model.hpp"
#include "speed_model.hpp"

#include "contour_conversion.hpp"
#include "py_array_conversion.hpp"
#include "py_validation.hpp"

namespace
{
constexpr auto kConnectivityDoc = R"(

Pixel connectivity used for contour extraction and topology handling.

Defines the neighborhood relationship between pixels in a 2D grid.

Attributes
----------
Four
    4-connected neighborhood.

    Each pixel is connected to its horizontal and vertical neighbors:

        left, right, top, bottom

    This connectivity is commonly used by active contour
    evolution and provides a more compact contour representation.

Eight
    8-connected neighborhood.

    Each pixel is connected to its horizontal, vertical,
    and diagonal neighbors.

    This connectivity produces more connected regions and
    is often preferred when extracting contours from binary masks.

Examples
--------
>>> contours = fluvel.find_contours(
...     mask,
...     connectivity=fluvel.Connectivity.Eight
... )

>>> contours = fluvel.find_contours(
...     mask,
...     connectivity=fluvel.Connectivity.Four
... )

)";

constexpr auto kFailureHandlingModeDoc = R"(

Failure handling strategies used by active contours.

Defines how the algorithm behaves when a degraded or
invalid state is detected during contour evolution.

Attributes
----------
StopOnFailure
    Stop the algorithm immediately when a failure occurs.

    This mode is recommended for image segmentation,
    where robustness and reproducibility are preferred
    over recovery attempts.

RecoverOnFailure
    Attempt to recover from a failure and continue the
    contour evolution.

    This mode is recommended for video tracking and
    temporal processing, where maintaining continuity
    may be preferable to stopping the algorithm.

)";

constexpr auto kColorSpaceOptionDoc = R"(

Color spaces supported by region-based models.

The selected color space is used to compute region statistics
and homogeneity terms during contour evolution.

Attributes
----------
RGB
    Red-Green-Blue color space.

    Suitable for most natural color images.

YUV
    Luminance-Chrominance color space.

    Separates intensity information from chromatic information.

Lab
    CIE Lab color space.

    Designed to better approximate human color perception.

Luv
    CIE Luv color space.

    Perceptually uniform color space commonly used for
    color analysis and segmentation tasks.

)";

constexpr auto kComponents3iDoc = R"(

Three-component integer container.

Used to define per-channel weights for region-based models.

The interpretation of each component depends on the selected
ColorSpaceOption:

- RGB: c1 = R, c2 = G, c3 = B
- YUV: c1 = Y, c2 = U, c3 = V
- Lab: c1 = L, c2 = a, c3 = b
- Luv: c1 = L, c2 = u, c3 = v

Larger values increase the contribution of the corresponding
channel to the region energy computation.

Parameters
----------
c1
    Weight of the first component.

c2
    Weight of the second component.

c3
    Weight of the third component.

Examples
--------
>>> weights = fluvel.Components3i(1, 1, 1)

>>> weights = fluvel.Components3i(2, 1, 1)

)";

constexpr auto kActiveContourParamsDoc = R"(

Parameters controlling active contour evolution.

This structure defines all parameters used during contour evolution,
including the data-driven phase (Cycle 1) and the optional smoothing
phase (Cycle 2).

Attributes
----------
cycle2_enabled : bool
    Enable the smoothing phase (Cycle 2).

    When enabled, the contour performs an additional smoothing
    evolution after the data-driven phase.

disk_radius : int
    Radius of the smoothing disk used during Cycle 2.

    Larger values produce stronger contour regularization.

Na : int
    Maximum number of iterations for Cycle 1.

    Cycle 1 is the data-driven evolution phase and is responsible
    for fitting the contour to image structures.

Ns : int
    Maximum number of iterations for Cycle 2.

    Cycle 2 performs contour smoothing and regularization.

failure_mode : FailureHandlingMode
    Strategy used when a degraded or invalid state is detected.

Defaults
--------
cycle2_enabled : True

disk_radius : 2

Na : 30

Ns : 3

failure_mode : FailureHandlingMode.StopOnFailure

Examples
--------
>>> params = fluvel.ActiveContourParams()

>>> params.Na = 50
>>> params.Ns = 5

>>> params.cycle2_enabled = False

>>> params.failure_mode = (
...     fluvel.FailureHandlingMode.RecoverOnFailure
... )

)";

constexpr auto kRegionModelParamsDoc = R"(

Parameters for region-based active contour models.

This structure defines the region homogeneity weights used by
Chan-Vese models and optional color-specific parameters for
multi-channel images.

Attributes
----------
lambda_in : int
    Weight of the inside homogeneity term (λ₁).

    Larger values increase the influence of the region inside
    the contour during energy minimization.

lambda_out : int
    Weight of the outside homogeneity term (λ₂).

    Larger values increase the influence of the region outside
    the contour during energy minimization.

color_space : ColorSpaceOption
    Color space used for region statistics and energy computation.

weights : Components3i
    Per-channel weights used during energy computation.

    The interpretation of each component depends on the selected
    color space:

    - RGB: c1 = R, c2 = G, c3 = B
    - YUV: c1 = Y, c2 = U, c3 = V
    - Lab: c1 = L, c2 = a, c3 = b
    - Luv: c1 = L, c2 = u, c3 = v

Defaults
--------
lambda_in : 1

lambda_out : 1

color_space : ColorSpaceOption.RGB

weights : Components3i(1, 1, 1)

Examples
--------
>>> params = fluvel.RegionModelParams()

>>> params.lambda_in = 2
>>> params.lambda_out = 1

>>> params.color_space = fluvel.ColorSpaceOption.Lab

>>> params.weights = fluvel.Components3i(2, 1, 1)

)";

constexpr auto kContourResultDoc = R"(

Contour extraction result.

Contains the inner and outer contour boundaries.

Attributes
----------
outer : numpy.ndarray
    Exterior contour boundary (Lout).

    Contains the points belonging to the outer boundary
    of the contour.

inner : numpy.ndarray
    Interior contour boundary (Lin).

    Contains the points belonging to the inner boundary
    of the contour.

Notes
-----
The contour arrays are returned as NumPy arrays of shape
(N, 2), where each row contains a point coordinate:

    [x, y]

Examples
--------
>>> result = fluvel.active_contour(image)

>>> outer = result.outer
>>> inner = result.inner

>>> contours = fluvel.find_contours(mask)

>>> outer = contours.outer
>>> inner = contours.inner

)";

constexpr auto kActiveContourDoc = R"(

Perform region-based active contour segmentation.

Evolves an active contour until a stopping condition is reached
using a region-based energy model.

The contour can be initialized automatically or from a user-provided
initialization mask.

Parameters
----------
image
    Input image.

phi_mask : numpy.ndarray, optional
    Initial contour mask.

    If omitted, the contour is initialized automatically
    using a centered ellipse.

    The ellipse covers approximately 80% of the image width
    and 80% of the image height.

    The mask must have the same dimensions as the input image.

    If a multi-channel image is provided, only the first channel
    is used.

active_contour_params : ActiveContourParams, optional
    Parameters controlling contour evolution.

region_model_params : RegionModelParams, optional
    Parameters controlling the region-based energy model.

Returns
-------
ContourResult
    Final contour boundaries after convergence.

Examples
--------
>>> result = fluvel.active_contour(image)

>>> params = fluvel.ActiveContourParams()
>>> params.Na = 50

>>> result = fluvel.active_contour(
...     image,
...     active_contour_params=params
... )

>>> region = fluvel.RegionModelParams()
>>> region.color_space = fluvel.ColorSpaceOption.Lab

>>> result = fluvel.active_contour(
...     image,
...     region_model_params=region
... )

>>> phi = np.zeros(image.shape[:2], dtype=np.uint8)

>>> result = fluvel.active_contour(
...     image,
...     phi_mask=phi
... )

Notes
-----
The algorithm automatically selects a grayscale or color
region model depending on the input image format.

)";

constexpr auto kFindContoursDoc = R"(

Extract contour boundaries from a binary mask.

Pixels greater than or equal to 128 are considered foreground.

Supported image layouts:

- Gray8
- BGR24
- BGRA32

For multi-channel images, only the first channel is used.

Parameters
----------
mask
    Binary mask image.

Returns
-------
ContourResult

Examples
--------
>>> contours = fluvel.find_contours(mask)

>>> outer = contours.outer
>>> inner = contours.inner

)";

struct ContourResult
{
    py::array outer;
    py::array inner;
};

} // namespace

void bindActiveContour(py::module_& m)
{
    py::enum_<fluvel_ip::Connectivity>(m, "Connectivity", kConnectivityDoc)
        .value("Four", fluvel_ip::Connectivity::Four)
        .value("Eight", fluvel_ip::Connectivity::Eight);

    py::enum_<fluvel_ip::FailureHandlingMode>(m, "FailureHandlingMode", kFailureHandlingModeDoc)
        .value("StopOnFailure", fluvel_ip::FailureHandlingMode::StopOnFailure)
        .value("RecoverOnFailure", fluvel_ip::FailureHandlingMode::RecoverOnFailure);

    py::enum_<fluvel_ip::ColorSpaceOption>(m, "ColorSpaceOption", kColorSpaceOptionDoc)
        .value("RGB", fluvel_ip::ColorSpaceOption::RGB)
        .value("YUV", fluvel_ip::ColorSpaceOption::YUV)
        .value("Lab", fluvel_ip::ColorSpaceOption::Lab)
        .value("Luv", fluvel_ip::ColorSpaceOption::Luv);

    py::class_<fluvel_ip::Components_3i>(m, "Components3i", kComponents3iDoc)
        .def(py::init(
                 [](int c1, int c2, int c3)
                 {
                     return fluvel_ip::Components_3i{c1, c2, c3};
                 }),
             py::arg("c1"), py::arg("c2"), py::arg("c3"))

        .def_readwrite("c1", &fluvel_ip::Components_3i::c1)
        .def_readwrite("c2", &fluvel_ip::Components_3i::c2)
        .def_readwrite("c3", &fluvel_ip::Components_3i::c3);

    py::class_<fluvel_ip::ActiveContourParams>(m, "ActiveContourParams", kActiveContourParamsDoc)
        .def(py::init<>())

        .def_readwrite("cycle2_enabled", &fluvel_ip::ActiveContourParams::cycle2Enabled)
        .def_readwrite("disk_radius", &fluvel_ip::ActiveContourParams::diskRadius)
        .def_readwrite("Na", &fluvel_ip::ActiveContourParams::Na)
        .def_readwrite("Ns", &fluvel_ip::ActiveContourParams::Ns)
        .def_readwrite("failure_mode", &fluvel_ip::ActiveContourParams::failureMode);

    py::class_<fluvel_ip::RegionColorParams>(m, "RegionModelParams", kRegionModelParamsDoc)
        .def(py::init<>())

        .def_readwrite("lambda_in", &fluvel_ip::RegionColorParams::lambdaIn)
        .def_readwrite("lambda_out", &fluvel_ip::RegionColorParams::lambdaOut)
        .def_readwrite("color_space", &fluvel_ip::RegionColorParams::colorSpace)
        .def_readwrite("weights", &fluvel_ip::RegionColorParams::weights);

    py::class_<ContourResult>(m, "ContourResult", kContourResultDoc)
        .def_readonly("outer", &ContourResult::outer)
        .def_readonly("inner", &ContourResult::inner);

    m.def(
        "active_contour",

        [](py::array_t<uint8_t> image, py::object phiMask,
           const fluvel_ip::ActiveContourParams& acParams,
           const fluvel_ip::RegionColorParams& regionParams)
        {
            validateImage(image);

            auto img = pyArrayToImageView(image);

            fluvel_ip::ContourData cd = [&]()
            {
                if (phiMask.is_none())
                    return fluvel_ip::ContourData(img.width(), img.height());

                auto phiMaskPyArray = phiMask.cast<py::array_t<uint8_t>>();
                validateImage(phiMaskPyArray);
                auto phiView = pyArrayToImageView(phiMaskPyArray);

                if (phiView.width() != img.width() || phiView.height() != img.height())
                    throw py::value_error("phi_mask must have the same dimensions as image");

                return fluvel_ip::ContourData(phiView);
            }();

            std::unique_ptr<fluvel_ip::ISpeedModel> model;

            if (img.format() == fluvel_ip::ImageFormat::Gray8)
                model = std::make_unique<fluvel_ip::RegionGraySpeedModel>(regionParams);
            else
                model = std::make_unique<fluvel_ip::RegionColorSpeedModel>(regionParams);

            fluvel_ip::ActiveContour ac(std::move(cd), std::move(model), acParams);

            ac.update(img);

            ac.converge();

            return ContourResult{contourToPyArray(ac.outerBoundary()),

                                 contourToPyArray(ac.innerBoundary())};
        },

        py::arg("image"), py::arg("phi_mask") = py::none(),
        py::arg("active_contour_params") = fluvel_ip::ActiveContourParams{},
        py::arg("region_model_params") = fluvel_ip::RegionColorParams{}, kActiveContourDoc);

    m.def(
        "find_contours",

        [](py::array_t<uint8_t> mask, fluvel_ip::Connectivity connectivity)
        {
            validateImage(mask);

            auto img = pyArrayToImageView(mask);

            fluvel_ip::ContourData cd(img, connectivity);

            return ContourResult{contourToPyArray(cd.outerBoundary()),
                                 contourToPyArray(cd.innerBoundary())};
        },

        py::arg("mask"), py::arg("connectivity") = fluvel_ip::Connectivity::Eight,
        kFindContoursDoc);
}