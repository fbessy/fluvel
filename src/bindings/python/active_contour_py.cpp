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

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

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
    Radius of the smoothing disk used during Cycle 2 (>= 1).

    Larger values produce stronger contour regularization.

Na : int
    Maximum number of iterations for Cycle 1 (>= 1).

    Cycle 1 is the data-driven evolution phase and is responsible
    for fitting the contour to image structures.

Ns : int
    Maximum number of iterations for Cycle 2 (>= 1 when
    the smoothing phase is enabled).

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

constexpr auto kContourSetDoc = R"(

Discrete contour representation.

Contains the inner and outer contour boundaries defined on a
rectangular grid.

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

grid_width : int
    Width of the grid on which the contour is defined.

grid_height : int
    Height of the grid on which the contour is defined.

mask : numpy.ndarray
    Binary mask of the region delimited by the contour,
    with shape (grid_height, grid_width) and dtype uint8.

    Pixels inside the contour (boundary included) are 255,
    all the others are 0. This is the segmentation result to
    use for region metrics (Dice, IoU) or to save as an image.

Notes
-----
The contour arrays are returned as NumPy arrays of shape
(N, 2), where each row contains a point coordinate:

    [x, y]

The points are the pixels of the boundary, in no particular
order: they cannot be used directly as a polygon. Use ``mask``
when a region is needed.

Examples
--------
>>> contours = fluvel.find_contours(mask)

>>> outer = contours.outer
>>> inner = contours.inner

>>> region = contours.mask

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
    Input image: uint8 array of shape (H, W), (H, W, 1),
    (H, W, 3) or (H, W, 4). Color channels are in BGR order
    (the order used by OpenCV). For an RGB image, reverse the
    channels first: ``image[..., ::-1]``.

phi_mask : numpy.ndarray, optional
    Initial contour mask (uint8, pixels >= 128 are inside).

    If omitted, or if the mask contains no pixel inside,
    the contour is initialized automatically
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
ContourSet
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

Supported masks: uint8 arrays of shape (H, W), (H, W, 1),
(H, W, 3) or (H, W, 4). For multi-channel masks, only the
first channel is used.

Parameters
----------
mask
    Binary mask image.

connectivity
    Pixel connectivity used to extract the contour.

Returns
-------
ContourSet
    ``outer`` and ``inner`` are empty arrays of shape (0, 2)
    when the mask has no foreground pixel. When the whole mask is
    foreground, ``outer`` is empty and ``inner`` contains the pixels
    of the image border.

Examples
--------
>>> contours = fluvel.find_contours(mask)

>>> outer = contours.outer
>>> inner = contours.inner

)";

// Outer and inner contours returned to Python.
struct ContourSet
{
    py::array_t<int> outer;
    py::array_t<int> inner;

    // Region inside the contour (255 inside, 0 outside), shape (H, W).
    py::array_t<uint8_t> mask;

    int gridWidth;
    int gridHeight;
};

// Result of a computation done without the GIL (no Python object inside).
struct RawContours
{
    fluvel_ip::ContourPoints outer;
    fluvel_ip::ContourPoints inner;

    std::vector<uint8_t> mask;

    int width{0};
    int height{0};
};

using PixelList = std::vector<std::array<int, 2>>;

py::array_t<int> pixelsToPyArray(const PixelList& pixels)
{
    py::array_t<int> out({static_cast<py::ssize_t>(pixels.size()), py::ssize_t(2)});

    auto view = out.mutable_unchecked<2>();

    for (size_t i = 0; i < pixels.size(); ++i)
    {
        view(i, 0) = pixels[i][0];
        view(i, 1) = pixels[i][1];
    }

    return out;
}

py::array_t<uint8_t> maskToPyArray(const std::vector<uint8_t>& mask, int width, int height)
{
    return py::array_t<uint8_t>({height, width}, mask.data());
}

// 255 inside the contour (boundary included), 0 outside.
std::vector<uint8_t> phiToMask(const fluvel_ip::DiscreteLevelSet& phi)
{
    std::vector<uint8_t> mask(static_cast<size_t>(phi.width()) * static_cast<size_t>(phi.height()));

    for (int y = 0; y < phi.height(); ++y)
    {
        for (int x = 0; x < phi.width(); ++x)
        {
            const auto value = phi.at(x, y);

            const bool inside = value == fluvel_ip::PhiValue::InsideRegion ||
                                value == fluvel_ip::PhiValue::InteriorBoundary;

            mask[static_cast<size_t>(y) * static_cast<size_t>(phi.width()) + static_cast<size_t>(x)] =
                inside ? 255 : 0;
        }
    }

    return mask;
}

ContourSet toContourSet(const RawContours& raw)
{
    return ContourSet{contourToPyArray(raw.outer), contourToPyArray(raw.inner),
                      maskToPyArray(raw.mask, raw.width, raw.height), raw.width, raw.height};
}

// Contours of a mask without any pixel inside, or entirely inside.
ContourSet trivialContourSet(int width, int height, bool allInside)
{
    PixelList inner;

    if (allInside)
    {
        // The object covers the image: its boundary is the image border.
        for (int x = 0; x < width; ++x)
        {
            inner.push_back({x, 0});

            if (height > 1)
                inner.push_back({x, height - 1});
        }

        for (int y = 1; y < height - 1; ++y)
        {
            inner.push_back({0, y});

            if (width > 1)
                inner.push_back({width - 1, y});
        }
    }

    const std::vector<uint8_t> mask(static_cast<size_t>(width) * static_cast<size_t>(height),
                                    allInside ? 255 : 0);

    return ContourSet{pixelsToPyArray({}), pixelsToPyArray(inner),
                      maskToPyArray(mask, width, height), width, height};
}

std::string boolToString(bool value)
{
    return value ? "True" : "False";
}

template <class Enum>
std::string enumName(Enum value)
{
    return std::string(py::str(py::cast(value).attr("name")));
}

std::string contourSetRepr(const ContourSet& set)
{
    return "ContourSet(outer=" + std::to_string(set.outer.shape(0)) +
           " points, inner=" + std::to_string(set.inner.shape(0)) +
           " points, grid=" + std::to_string(set.gridWidth) + "x" + std::to_string(set.gridHeight) +
           ")";
}

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
        .def_readwrite("c3", &fluvel_ip::Components_3i::c3)

        .def("__repr__",
             [](const fluvel_ip::Components_3i& c)
             {
                 return "Components3i(" + std::to_string(c.c1) + ", " + std::to_string(c.c2) +
                        ", " + std::to_string(c.c3) + ")";
             });

    py::class_<fluvel_ip::ActiveContourParams>(m, "ActiveContourParams", kActiveContourParamsDoc)
        .def(py::init<>())

        .def_readwrite("cycle2_enabled", &fluvel_ip::ActiveContourParams::cycle2Enabled)
        .def_readwrite("disk_radius", &fluvel_ip::ActiveContourParams::diskRadius)
        .def_readwrite("Na", &fluvel_ip::ActiveContourParams::Na)
        .def_readwrite("Ns", &fluvel_ip::ActiveContourParams::Ns)
        .def_readwrite("failure_mode", &fluvel_ip::ActiveContourParams::failureMode)

        .def("__repr__",
             [](const fluvel_ip::ActiveContourParams& p)
             {
                 return "ActiveContourParams(cycle2_enabled=" + boolToString(p.cycle2Enabled) +
                        ", disk_radius=" + std::to_string(p.diskRadius) +
                        ", Na=" + std::to_string(p.Na) + ", Ns=" + std::to_string(p.Ns) +
                        ", failure_mode=" + enumName(p.failureMode) + ")";
             });

    py::class_<fluvel_ip::RegionColorParams>(m, "RegionModelParams", kRegionModelParamsDoc)
        .def(py::init<>())

        .def_readwrite("lambda_in", &fluvel_ip::RegionColorParams::lambdaIn)
        .def_readwrite("lambda_out", &fluvel_ip::RegionColorParams::lambdaOut)
        .def_readwrite("color_space", &fluvel_ip::RegionColorParams::colorSpace)
        .def_readwrite("weights", &fluvel_ip::RegionColorParams::weights)

        .def("__repr__",
             [](const fluvel_ip::RegionColorParams& p)
             {
                 return "RegionModelParams(lambda_in=" + std::to_string(p.lambdaIn) +
                        ", lambda_out=" + std::to_string(p.lambdaOut) +
                        ", color_space=" + enumName(p.colorSpace) + ", weights=Components3i(" +
                        std::to_string(p.weights.c1) + ", " + std::to_string(p.weights.c2) + ", " +
                        std::to_string(p.weights.c3) + "))";
             });

    py::class_<ContourSet>(m, "ContourSet", kContourSetDoc)
        .def_readonly("outer", &ContourSet::outer)
        .def_readonly("inner", &ContourSet::inner)
        .def_readonly("mask", &ContourSet::mask)
        .def_readonly("grid_width", &ContourSet::gridWidth)
        .def_readonly("grid_height", &ContourSet::gridHeight)
        .def("__repr__", &contourSetRepr);

    m.def(
        "active_contour",

        [](const py::array& image, const py::object& phiMask,
           const fluvel_ip::ActiveContourParams& acParams,
           const fluvel_ip::RegionColorParams& regionParams)
        {
            const auto imageArray = toImageArray(image);

            validateActiveContourParams(acParams);
            validateRegionParams(regionParams);

            const auto img = pyArrayToImageView(imageArray);

            // Both arrays must stay alive while the views are used.
            std::optional<PyImageArray> phiArray;

            if (!phiMask.is_none())
            {
                if (!py::isinstance<py::array>(phiMask))
                    throw py::type_error("phi_mask must be a NumPy array or None");

                phiArray = toImageArray(phiMask.cast<py::array>());
            }

            std::optional<fluvel_ip::ImageView> phiView;

            if (phiArray)
            {
                phiView = pyArrayToImageView(*phiArray);

                if (phiView->width() != img.width() || phiView->height() != img.height())
                    throw py::value_error("phi_mask must have the same dimensions as image");
            }

            // The computation does not need Python: other threads can run meanwhile.
            const RawContours raw = runWithoutGil(
                [&]()
                {
                    fluvel_ip::ContourData cd = phiView
                                                    ? fluvel_ip::ContourData(*phiView)
                                                    : fluvel_ip::ContourData(img.width(), img.height());

                    std::unique_ptr<fluvel_ip::ISpeedModel> model;

                    if (img.format() == fluvel_ip::ImageFormat::Gray8)
                        model = std::make_unique<fluvel_ip::RegionGraySpeedModel>(regionParams);
                    else
                        model = std::make_unique<fluvel_ip::RegionColorSpeedModel>(regionParams);

                    fluvel_ip::ActiveContour ac(std::move(cd), std::move(model), acParams);

                    ac.update(img);

                    ac.converge();

                    return RawContours{ac.outerBoundary(), ac.innerBoundary(), phiToMask(ac.phi()),
                                       ac.phi().width(), ac.phi().height()};
                });

            return toContourSet(raw);
        },

        py::arg("image"), py::arg("phi_mask") = py::none(),
        py::arg("active_contour_params") = fluvel_ip::ActiveContourParams{},
        py::arg("region_model_params") = fluvel_ip::RegionColorParams{}, kActiveContourDoc);

    m.def(
        "find_contours",

        [](const py::array& mask, fluvel_ip::Connectivity connectivity)
        {
            const auto maskArray = toImageArray(mask);

            const auto img = pyArrayToImageView(maskArray);

            const int width = img.width();
            const int height = img.height();

            size_t foreground = 0;

            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    if (img.at(x, y) >= 128u)
                        ++foreground;
                }
            }

            // ContourData falls back to a default ellipse when the mask has no
            // boundary: an empty or full mask must not turn into a fake object.
            const size_t total = static_cast<size_t>(width) * static_cast<size_t>(height);

            if (foreground == 0 || foreground == total)
                return trivialContourSet(width, height, foreground == total);

            const RawContours raw = runWithoutGil(
                [&]()
                {
                    fluvel_ip::ContourData cd(img, connectivity);

                    return RawContours{cd.outerBoundary(), cd.innerBoundary(), phiToMask(cd.phi()),
                                       cd.phi().width(), cd.phi().height()};
                });

            return toContourSet(raw);
        },

        py::arg("mask"), py::arg("connectivity") = fluvel_ip::Connectivity::Eight,
        kFindContoursDoc);
}
