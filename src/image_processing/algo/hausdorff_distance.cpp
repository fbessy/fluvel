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
#include <cstddef>
#include <numeric>

#include "fluvel_math.hpp"

namespace fluvel_ip
{

constexpr size_t kInitialArrayAllocSize = 10000u;

//! Constructor. The third parameter is the intersection between shape a and b, i.e.
//! the points (offsets) in common. It's an optional parameter. It can speed up
//! the computation (proportionally of the intersection part) in case of the intersection is
//! easily to compute in constant time ( complexity in 0(1) ), with an image or a matrix.
HausdorffDistance::HausdorffDistance(const Shape& shapeA, const Shape& shapeB,
                                     const PointSet* intersectionAB)
    : shapeA_(shapeA)
    , shapeB_(shapeB)
    , intersectionAB_(intersectionAB)
{
    minDistancesFromAToB_.reserve(kInitialArrayAllocSize);
    minDistancesFromBToA_.reserve(kInitialArrayAllocSize);

    compute();
}

void HausdorffDistance::compute()
{
    if (shapeA_.isValid() && shapeB_.isValid())
    {
#if defined(RANDOM_SAMPLING) && !defined(NAIVE_ALGO)
        shapeA_.shufflePoints();
        shapeB_.shufflePoints();
#endif

        // compute the directed or relative hausdorff distance in the both directions
        computeDirectedHausdorff(shapeA_, shapeB_, distanceFromAToB_, minDistancesFromAToB_);

        computeDirectedHausdorff(shapeB_, shapeA_, distanceFromBToA_, minDistancesFromBToA_);
    }
}

void HausdorffDistance::computeDirectedHausdorff(const Shape& fromShape, const Shape& toShape,
                                                 float& directedDistance,
                                                 std::vector<float>& minDistances)
{
    Point2D_f fromPointRel, toPointRel;

    float dist, minDist;

    // initialization with a minimum value in order to maximize
    // the directed or relative hausdorff distance
    directedDistance = 0.f;

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
            minDist = std::numeric_limits<float>::max();

            // inner loop
            for (const auto& toPoint : toShape.points())
            {
                toPointRel.x = float(toPoint.x) - toShape.centroid().x;
                toPointRel.y = float(toPoint.y) - toShape.centroid().y;

                dist = math::euclideanDistance(fromPointRel, toPointRel);

                // it minimizes minDist
                if (dist < minDist)
                {
                    minDist = dist;
                }

#if defined(EARLY_BREAKING) && !defined(NAIVE_ALGO)
                if (minDist < directedDistance)
                {
                    break;
                }
#endif
            }

            minDistances.push_back(minDist);

            // it maximizes the relative or directed hausdorff distance
            if (minDist > directedDistance)
            {
                directedDistance = minDist;
            }

#if defined(INTERSECTION_EXCLUSION) && !defined(NAIVE_ALGO)
        }
#endif
    }

    // This empty case is possible if shape_a and shape_b are identical
    // with the optimization INTERSECTION_EXCLUSION and
    // with intersection_a_b with all points.
    // In this case, the inner loop is never performed,
    // even if shapes are not empty.
    // It forces to be able to pick up one coherent value
    // instead of float max.
    if (minDistances.empty() && !fromShape.points().empty() && !toShape.points().empty())
        minDistances.push_back(0.f);
}

float HausdorffDistance::distance() const
{
    return std::max(distanceFromAToB_, distanceFromBToA_);
}

float HausdorffDistance::modifiedDistance() const
{
    return std::max(calculateMean(minDistancesFromAToB_), calculateMean(minDistancesFromBToA_));
}

float HausdorffDistance::hausdorffQuantile(int hundredth)
{
    if (!isSorted_)
    {
        std::ranges::sort(minDistancesFromAToB_);
        std::ranges::sort(minDistancesFromBToA_);

        isSorted_ = true;
    }

    return std::max(hausdorffQuantile(minDistancesFromAToB_, hundredth),
                    hausdorffQuantile(minDistancesFromBToA_, hundredth));
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
