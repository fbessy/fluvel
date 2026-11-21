// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "hausdorff_py.hpp"
#include "contour_conversion.hpp"
#include "hausdorff_distance.hpp"
#include "hausdorff_statistics.hpp"

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
    Hausdorff distance (maximum of the two directed distances).

modified_distance
    Modified Hausdorff distance (mean of the minimum distances,
    maximum of the two directions).

quantile_distance
    Quantile Hausdorff distance. The quantile is taken in each
    direction (nearest rank, lower index) and the larger value is kept.

centroids_distance
    Distance between contour centroids.

Notes
-----
All the distances are exact. They are measured in absolute pixel
coordinates unless ``align_centroids=True`` was requested.

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

align_centroids
    If False (default), distances are measured in absolute pixel
    coordinates, as in most libraries and papers: a translation
    between the contours increases the distance.

    If True, the centroids of both contours are aligned first, so
    only the difference of shape is measured; the translation is
    then given by ``centroids_distance``. This is what the Fluvel Set
    application can do in its analysis window.

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

        [](const py::array_t<int>& contourA, const py::array_t<int>& contourB, int quantile,
           bool alignCentroids)
        {
            if (quantile < 0 || quantile > 100)
                throw py::value_error("quantile must be in the range [0, 100]");

            auto shapeA = pyArrayToShape(contourA);
            auto shapeB = pyArrayToShape(contourB);

            const fluvel_ip::HausdorffOptions options{.alignCentroids = alignCentroids};

            const fluvel_ip::HausdorffStatistics stats(shapeA, shapeB, nullptr, options);
            const fluvel_ip::HausdorffDistance hd(std::move(shapeA), std::move(shapeB), nullptr,
                                                  options);

            return HausdorffMetricsResult{hd.distance(), stats.modifiedDistance(),
                                          stats.quantileDistance(quantile),
                                          hd.centroidsDistance()};
        },

        py::arg("contour_a"), py::arg("contour_b"), py::arg("quantile") = 95,
        py::arg("align_centroids") = false, kHausdorffMetricsDoc);
}