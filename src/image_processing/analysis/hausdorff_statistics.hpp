// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "hausdorff_distance.hpp"
#include "point_containers.hpp"
#include "shape.hpp"

#include <vector>

namespace fluvel_ip
{

/**
 * @brief Exact statistics of the point-to-shape distances of two shapes.
 *
 * For each point of a shape, the distance to the other shape is the minimum
 * distance to the points of that shape. This class computes all these
 * distances exactly (no early breaking), in both directions (A → B and
 * B → A), and provides:
 * - the modified Hausdorff distance (mean of the distances),
 * - the quantile Hausdorff distance (e.g. HD95), more robust to outliers
 *   than the maximum.
 *
 * Both values are exact. The algorithm of Taha and Hanbury used by
 * HausdorffDistance cannot provide them, because it keeps only partial
 * minima, which overestimate the mean and the quantiles without any bound.
 *
 * The cost is O(n × m) for shapes of n and m points, paid once in the
 * constructor; the getters are then cheap.
 */
class HausdorffStatistics
{
public:
    /**
     * @brief Construct the object and compute all the distances.
     *
     * @param shapeA First shape.
     * @param shapeB Second shape.
     * @param intersectionAB Optional set of points shared by both shapes. Such
     *        points are at distance 0 and are not searched. It is ignored when
     *        the centroids are aligned.
     * @param options Measurement options, see HausdorffOptions.
     */
    HausdorffStatistics(const Shape& shapeA, const Shape& shapeB,
                        const PointSet* intersectionAB = nullptr, HausdorffOptions options = {});

    /**
     * @brief Modified Hausdorff distance.
     *
     * Mean of the minimum distances in each direction; the larger of the
     * two means is returned.
     *
     * @return Distance in pixels, or NaN if a shape is not valid.
     */
    [[nodiscard]]
    float modifiedDistance() const;

    /**
     * @brief Quantile Hausdorff distance.
     *
     * The quantile of the minimum distances is taken in each direction with the
     * nearest-rank rule (lower index, ((n - 1) * h) / 100), and the larger of
     * the two is returned. 100 gives the Hausdorff distance, 0 the smallest
     * minimum distance.
     *
     * @param hundredth Percentile in [0, 100] (clamped).
     * @return Distance in pixels, or NaN if a shape is not valid.
     */
    [[nodiscard]]
    float quantileDistance(int hundredth) const;

private:
    /**
     * @brief Exact minimum distance of every point of fromShape to toShape, sorted.
     */
    [[nodiscard]]
    std::vector<float> sortedMinima(const Shape& fromShape, const Shape& toShape) const;

    [[nodiscard]]
    static float mean(const std::vector<float>& values);

    [[nodiscard]]
    static float quantile(const std::vector<float>& sortedValues, int hundredth);

    const PointSet* intersectionAB_{nullptr};

    HausdorffOptions options_{};

    std::vector<float> minimaAToB_; ///< Sorted minimum distances, A → B.
    std::vector<float> minimaBToA_; ///< Sorted minimum distances, B → A.
};

} // namespace fluvel_ip
