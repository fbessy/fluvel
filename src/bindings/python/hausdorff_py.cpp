// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "hausdorff_py.hpp"
#include "contour_conversion.hpp"
#include "hausdorff_distance.hpp"

#include <pybind11/numpy.h>

#include <string>
#include <utility>

namespace
{

struct HausdorffMetricsResult
{
    float distance;
    float modifiedDistance;
    float quantileDistance;
    float centroidsDistance;
};

constexpr auto kHausdorffMetricsResultDoc = R"(

Hausdorff distance metrics between two contours.

Attributes
----------
distance
    Hausdorff distance, computed after aligning the centroids
    of the two contours: a translation between the contours
    does not change it (it is given by ``centroids_distance``).

modified_distance
    Modified Hausdorff distance (mean of the minimum distances),
    computed after aligning the centroids.

quantile_distance
    Quantile Hausdorff distance, computed after aligning
    the centroids.

centroids_distance
    Distance between contour centroids.

Notes
-----
``modified_distance`` and ``quantile_distance`` may be
overestimated: the algorithm used to speed up the computation
is exact only for ``distance``.

)";

constexpr auto kHausdorffMetricsDoc = R"(

Compute Hausdorff distance metrics between two contours.

Parameters
----------
contour_a
    First contour.

contour_b
    Second contour.

quantile
    Quantile used for quantile Hausdorff distance.

    Typical values are:

    - 95 for HD95
    - 90 for HD90
    - 80 for HD80

Returns
-------
HausdorffMetricsResult

)";

} // namespace

void bindHausdorff(py::module_& m)
{
    py::class_<HausdorffMetricsResult>(m, "HausdorffMetricsResult", kHausdorffMetricsResultDoc)
        .def_readonly("distance", &HausdorffMetricsResult::distance)
        .def_readonly("modified_distance", &HausdorffMetricsResult::modifiedDistance)
        .def_readonly("quantile_distance", &HausdorffMetricsResult::quantileDistance)
        .def_readonly("centroids_distance", &HausdorffMetricsResult::centroidsDistance)
        .def("__repr__",
             [](const HausdorffMetricsResult& r)
             {
                 return "HausdorffMetricsResult(distance=" + std::to_string(r.distance) +
                        ", modified_distance=" + std::to_string(r.modifiedDistance) +
                        ", quantile_distance=" + std::to_string(r.quantileDistance) +
                        ", centroids_distance=" + std::to_string(r.centroidsDistance) + ")";
             });

    m.def(
        "hausdorff_metrics",

        [](const py::array_t<int>& contourA, const py::array_t<int>& contourB, int quantile)
        {
            if (quantile < 0 || quantile > 100)
                throw py::value_error("quantile must be in the range [0, 100]");

            auto shapeA = pyArrayToShape(contourA);
            auto shapeB = pyArrayToShape(contourB);

            fluvel_ip::HausdorffDistance hd(std::move(shapeA), std::move(shapeB));

            return HausdorffMetricsResult{hd.distance(), hd.modifiedDistance(),
                                          hd.hausdorffQuantile(quantile), hd.centroidsDistance()};
        },

        py::arg("contour_a"), py::arg("contour_b"), py::arg("quantile") = 95, kHausdorffMetricsDoc);
}