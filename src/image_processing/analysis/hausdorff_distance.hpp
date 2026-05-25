// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "point_containers.hpp"
#include "shape.hpp"

#include <vector>

namespace fluvel_ip
{

/**
 * @brief Compute Hausdorff-based distances between two shapes.
 *
 * This class provides:
 * - the exact Hausdorff distance,
 * - a modified (mean-based) Hausdorff distance,
 * - a quantile-based Hausdorff distance,
 * - the centroid distance between shapes.
 *
 * It internally computes directed distances (A → B and B → A),
 * and stores intermediate minimum distances for reuse.
 *
 * @note An optional intersection set can be provided to speed up
 *       computations (common points between shapes).
 */
class HausdorffDistance
{
public:
    /**
     * @brief Construct a HausdorffDistance object.
     *
     * @param shapeA First shape.
     * @param shapeB Second shape.
     * @param intersectionAB Optional set of common points between A and B.
     */
    HausdorffDistance(const Shape& shapeA, const Shape& shapeB,
                      const PointSet* intersectionAB = nullptr);

    /**
     * @brief Compute the symmetric Hausdorff distance.
     *
     * Defined as:
     * max( d(A → B), d(B → A) )
     *
     * @return Hausdorff distance.
     */
    float distance() const;

    /**
     * @brief Compute the modified Hausdorff distance.
     *
     * Typically based on the mean of minimum distances instead of the maximum.
     *
     * @note The result may be overestimated if EARLY_BREAKING optimization is used.
     *
     * @return Modified Hausdorff distance.
     */
    float modifiedDistance() const;

    /**
     * @brief Compute the Hausdorff distance quantile.
     *
     * Instead of taking the maximum, this returns a percentile of the
     * distribution of minimum distances.
     *
     * @param hundredth Percentile in [0,100].
     * @return Quantile Hausdorff distance.
     */
    float hausdorffQuantile(int hundredth);

    /**
     * @brief Compute the distance between centroids of both shapes.
     *
     * @return Euclidean distance between centroids.
     */
    float centroidsDistance() const;

private:
    /**
     * @brief Compute all Hausdorff-related distances.
     *
     * Populates directed distances and minimum distance arrays.
     */
    void compute();

    /**
     * @brief Compute directed Hausdorff distance.
     *
     * For each point in @p fromShape, computes the minimum distance
     * to points in @p toShape.
     *
     * @param fromShape Source shape.
     * @param toShape Target shape.
     * @param directedDistance Output maximum of minimum distances.
     * @param minDistances Output vector of minimum distances.
     */
    void computeDirectedHausdorff(const Shape& fromShape, const Shape& toShape,
                                  float& directedDistance, std::vector<float>& minDistances);

    /**
     * @brief Compute the mean of minimum distances.
     *
     * @param minDists Vector of minimum distances.
     * @return Mean value.
     */
    static float calculateMean(const std::vector<float>& minDists);

    /**
     * @brief Compute a quantile from minimum distances.
     *
     * @param minDists Vector of minimum distances.
     * @param hundredth Percentile in [0,100].
     * @return Quantile value.
     */
    static float hausdorffQuantile(const std::vector<float>& minDists, int hundredth);

    Shape shapeA_; ///< First shape (point set representation).
    Shape shapeB_; ///< Second shape (point set representation).

    /**
     * @brief Optional intersection between shapes A and B.
     *
     * Used to speed up distance computation.
     */
    const PointSet* intersectionAB_{nullptr};

    /**
     * @brief Directed Hausdorff distance from A to B.
     */
    float distanceFromAToB_{0.f};

    /**
     * @brief Directed Hausdorff distance from B to A.
     */
    float distanceFromBToA_{0.f};

    /**
     * @brief Minimum distances from A to B.
     */
    std::vector<float> minDistancesFromAToB_;

    /**
     * @brief Minimum distances from B to A.
     */
    std::vector<float> minDistancesFromBToA_;

    /**
     * @brief Indicates whether minimum distance arrays are sorted.
     *
     * Used for quantile computation optimization.
     */
    bool isSorted_{false};
};

} // namespace fluvel_ip
