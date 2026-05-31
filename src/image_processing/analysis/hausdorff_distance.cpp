// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "hausdorff_distance.hpp"

// #define NAIVE_ALGO

// According to
// "An Efficient Algorithm for Calculating
// the Exact Hausdorff Distance"
// Abdel Aziz Taha, Allan Hanbury, IEEE. PAMI, 2015

#ifndef NAIVE_ALGO

#define EARLY_BREAKING         // ×20 faster
#define RANDOM_SAMPLING        // ×40 faster with early breaking
#define INTERSECTION_EXCLUSION // data dependant and proportional
                               // to the intersection part
#endif

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <utility>

#include "fluvel_math.hpp"

namespace fluvel_ip
{

constexpr size_t kInitialArrayAllocSize = 10000u;

HausdorffDistance::HausdorffDistance(Shape shapeA, Shape shapeB, const PointSet* intersectionAB)
    : shapeA_(std::move(shapeA))
    , shapeB_(std::move(shapeB))
    , intersectionAB_(intersectionAB)
{
    minDistancesFromAToB_.reserve(kInitialArrayAllocSize);
    minDistancesFromBToA_.reserve(kInitialArrayAllocSize);

    compute();
}

void HausdorffDistance::compute()
{
    if (!shapeA_.isValid() || !shapeB_.isValid())
        return;

#if defined(RANDOM_SAMPLING) && !defined(NAIVE_ALGO)
    shapeA_.shufflePoints();
    shapeB_.shufflePoints();
#endif

    // compute the directed or relative hausdorff distance in the both directions
    computeDirectedHausdorff(shapeA_, shapeB_, distanceFromAToB_, minDistancesFromAToB_);

    computeDirectedHausdorff(shapeB_, shapeA_, distanceFromBToA_, minDistancesFromBToA_);
}

void HausdorffDistance::computeDirectedHausdorff(const Shape& fromShape, const Shape& toShape,
                                                 float& directedDistance,
                                                 std::vector<float>& minDistances)
{
    Point2D_f fromPointRel, toPointRel;

    float directedDist = std::numeric_limits<float>::quiet_NaN();

    // outer loop
    for (const auto& fromPoint : fromShape.points())
    {
        // intersection_a_b is optional and can be empty by default,
        // cf constructor documentation.
        // It checks in constant time or O(1) complexity with the method contains
        // due to the stl unordered set container.

#if defined(INTERSECTION_EXCLUSION) && !defined(NAIVE_ALGO)
        if (intersectionAB_ && !intersectionAB_->contains(fromPoint))
        {
#endif
            fromPointRel.x = float(fromPoint.x) - fromShape.centroid().x;
            fromPointRel.y = float(fromPoint.y) - fromShape.centroid().y;

            // initialization in order to minimize
            float minDist = std::numeric_limits<float>::max();

            // inner loop
            for (const auto& toPoint : toShape.points())
            {
                toPointRel.x = float(toPoint.x) - toShape.centroid().x;
                toPointRel.y = float(toPoint.y) - toShape.centroid().y;

                const float currentDist = math::euclideanDistance(fromPointRel, toPointRel);

                // it minimizes minDist
                if (currentDist < minDist)
                    minDist = currentDist;

#if defined(EARLY_BREAKING) && !defined(NAIVE_ALGO)
                if (!std::isnan(directedDist) && minDist < directedDist)
                    break;
#endif
            }

            minDistances.push_back(minDist);

            // it maximizes the relative or directed hausdorff distance
            if (std::isnan(directedDist) || minDist > directedDist)
                directedDist = minDist;

#if defined(INTERSECTION_EXCLUSION) && !defined(NAIVE_ALGO)
        }
#endif
    }

    // When all points belong to the provided intersection,
    // no distance is evaluated.
    // In this case the Hausdorff distance is zero because
    // both shapes are identical on the evaluated domain.
    if (minDistances.empty() && !fromShape.points().empty() && !toShape.points().empty())
    {
        minDistances.push_back(0.f);

        directedDistance = 0.f;
    }
    else
    {
        directedDistance = directedDist;
    }
}

float HausdorffDistance::distance() const
{
    if (std::isnan(distanceFromAToB_) || std::isnan(distanceFromBToA_))
        return std::numeric_limits<float>::quiet_NaN();

    return std::max(distanceFromAToB_, distanceFromBToA_);
}

float HausdorffDistance::modifiedDistance() const
{
    const float meanAToB = calculateMean(minDistancesFromAToB_);
    const float meanBToA = calculateMean(minDistancesFromBToA_);

    if (std::isnan(meanAToB) || std::isnan(meanBToA))
        return std::numeric_limits<float>::quiet_NaN();

    return std::max(meanAToB, meanBToA);
}

float HausdorffDistance::hausdorffQuantile(int hundredth)
{
    if (!isSorted_)
    {
        std::ranges::sort(minDistancesFromAToB_);
        std::ranges::sort(minDistancesFromBToA_);

        isSorted_ = true;
    }

    const float quantileAToB = hausdorffQuantile(minDistancesFromAToB_, hundredth);
    const float quantileBToA = hausdorffQuantile(minDistancesFromBToA_, hundredth);

    if (std::isnan(quantileAToB) || std::isnan(quantileBToA))
        return std::numeric_limits<float>::quiet_NaN();

    return std::max(quantileAToB, quantileBToA);
}

float HausdorffDistance::calculateMean(const std::vector<float>& minDists)
{
    if (minDists.empty())
        return std::numeric_limits<float>::quiet_NaN();

    const float sum = std::accumulate(minDists.begin(), minDists.end(), 0.f);

    return sum / static_cast<float>(minDists.size());
}

float HausdorffDistance::hausdorffQuantile(const std::vector<float>& minDists, int hundredth)
{
    if (minDists.empty())
        return std::numeric_limits<float>::quiet_NaN();

    const int h = std::clamp(hundredth, 0, 100);

    const int idx = ((int(minDists.size()) - 1) * h) / 100;

    return minDists[idx];
}

float HausdorffDistance::centroidsDistance() const
{
    if (!shapeA_.isValid() || !shapeB_.isValid())
        return std::numeric_limits<float>::quiet_NaN();

    return math::euclideanDistance(shapeA_.centroid(), shapeB_.centroid());
}

} // namespace fluvel_ip
