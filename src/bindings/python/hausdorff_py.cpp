// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "hausdorff_py.hpp"
#include "contour_conversion.hpp"
#include "hausdorff_distance.hpp"

#include <pybind11/numpy.h>

#include <utility>

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
    Standard Hausdorff distance.

modified_distance
    Modified Hausdorff distance.

quantile_distance
    Quantile Hausdorff distance.

centroids_distance
    Distance between contour centroids.

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

void bindHausdorff(py::module_& m)
{
    py::class_<HausdorffMetricsResult>(m, "HausdorffMetricsResult", kHausdorffMetricsResultDoc)
        .def_readonly("distance", &HausdorffMetricsResult::distance)
        .def_readonly("modified_distance", &HausdorffMetricsResult::modifiedDistance)
        .def_readonly("quantile_distance", &HausdorffMetricsResult::quantileDistance)
        .def_readonly("centroids_distance", &HausdorffMetricsResult::centroidsDistance);

    m.def(
        "hausdorff_metrics",

        [](const py::array_t<int>& contourA, const py::array_t<int>& contourB, int quantile)
        {
            auto shapeA = pyArrayToShape(contourA);
            auto shapeB = pyArrayToShape(contourB);

            fluvel_ip::HausdorffDistance hd(std::move(shapeA), std::move(shapeB));

            return HausdorffMetricsResult{hd.distance(), hd.modifiedDistance(),
                                          hd.hausdorffQuantile(quantile), hd.centroidsDistance()};
        },

        py::arg("contour_a"), py::arg("contour_b"), py::arg("quantile") = 95, kHausdorffMetricsDoc);
}