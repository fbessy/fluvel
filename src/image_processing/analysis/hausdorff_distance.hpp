// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "point_containers.hpp"
#include "shape.hpp"

#include <limits>

namespace fluvel_ip
{

/**
 * @brief Options shared by the Hausdorff-based measures.
 */
struct HausdorffOptions
{
    /**
     * @brief Align the centroids of both shapes before measuring distances.
     *
     * When false (default), distances are measured in absolute pixel
     * coordinates, which is the usual definition (MONAI, MedPy, SimpleITK...):
     * a translation between the shapes increases the distance.
     *
     * When true, every point is expressed relatively to the centroid of its
     * own shape, so only the difference of shape is measured. The translation
     * is available through HausdorffDistance::centroidsDistance().
     */
    bool alignCentroids{false};
};

/**
 * @brief Exact Hausdorff distance between two shapes.
 *
 * Computes the symmetric Hausdorff distance, max(d(A → B), d(B → A)),
 * with the algorithm of Taha and Hanbury (IEEE PAMI, 2015), which is
 * exact for the maximum:
 * - early breaking of the inner loop,
 * - random sampling of the points,
 * - exclusion of the points shared by both shapes (when coordinates are absolute).
 *
 * @note This algorithm discards most of the minimum distances it evaluates.
 *       For the mean or the quantiles of these distances, use
 *       HausdorffStatistics, which is exact too.
 */
class HausdorffDistance
{
public:
    /**
     * @brief Construct the object and compute the distance.
     *
     * The shapes are copied or moved into the object.
     *
     * @param shapeA First shape.
     * @param shapeB Second shape.
     * @param intersectionAB Optional set of points shared by both shapes. Such
     *        points are at distance 0, so they are not evaluated, which speeds
     *        up the computation when the shapes overlap. It is ignored when
     *        the centroids are aligned, since a shared pixel is then no longer
     *        at distance 0.
     * @param options Measurement options, see HausdorffOptions.
     */
    HausdorffDistance(Shape shapeA, Shape shapeB, const PointSet* intersectionAB = nullptr,
                      HausdorffOptions options = {});

    /**
     * @brief Symmetric Hausdorff distance, in pixels.
     *
     * @return max( d(A → B), d(B → A) ), or NaN if a shape is not valid.
     */
    [[nodiscard]]
    float distance() const;

    /**
     * @brief Euclidean distance between the centroids of both shapes, in pixels.
     *
     * @return Distance, or NaN if a shape is not valid.
     */
    [[nodiscard]]
    float centroidsDistance() const;

private:
    /**
     * @brief Directed Hausdorff distance from one shape to the other.
     */
    [[nodiscard]]
    float computeDirected(const Shape& fromShape, const Shape& toShape) const;

    Shape shapeA_;
    Shape shapeB_;

    const PointSet* intersectionAB_{nullptr};

    HausdorffOptions options_{};

    float distanceFromAToB_{std::numeric_limits<float>::quiet_NaN()};
    float distanceFromBToA_{std::numeric_limits<float>::quiet_NaN()};
};

} // namespace fluvel_ip
