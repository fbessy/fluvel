// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "hausdorff_distance.hpp"

// According to
// "An Efficient Algorithm for Calculating
// the Exact Hausdorff Distance"
// Abdel Aziz Taha, Allan Hanbury, IEEE. PAMI, 2015

#include <algorithm>
#include <cmath>
#include <utility>

#include "fluvel_math.hpp"

namespace fluvel_ip
{

HausdorffDistance::HausdorffDistance(Shape shapeA, Shape shapeB, const PointSet* intersectionAB,
                                     HausdorffOptions options)
    : shapeA_(std::move(shapeA))
    , shapeB_(std::move(shapeB))
    , intersectionAB_(options.alignCentroids ? nullptr : intersectionAB)
    , options_(options)
{
    if (!shapeA_.isValid() || !shapeB_.isValid())
        return;

    // Random sampling: shuffling the points makes the early breaking
    // efficient whatever the order of the points in the input.
    shapeA_.shufflePoints();
    shapeB_.shufflePoints();

    distanceFromAToB_ = computeDirected(shapeA_, shapeB_);
    distanceFromBToA_ = computeDirected(shapeB_, shapeA_);
}

float HausdorffDistance::computeDirected(const Shape& fromShape, const Shape& toShape) const
{
    const Point2D_f fromOrigin =
        options_.alignCentroids ? fromShape.centroid() : Point2D_f{0.f, 0.f};
    const Point2D_f toOrigin = options_.alignCentroids ? toShape.centroid() : Point2D_f{0.f, 0.f};

    float directedDist = std::numeric_limits<float>::quiet_NaN();

    for (const auto& fromPoint : fromShape.points())
    {
        // Intersection exclusion: a shared point is at distance 0 and cannot
        // raise the maximum. The lookup is O(1) (unordered set).
        if (intersectionAB_ && intersectionAB_->contains(fromPoint))
            continue;

        const Point2D_f from{float(fromPoint.x) - fromOrigin.x,
                             float(fromPoint.y) - fromOrigin.y};

        float minDist = std::numeric_limits<float>::max();

        for (const auto& toPoint : toShape.points())
        {
            const Point2D_f to{float(toPoint.x) - toOrigin.x, float(toPoint.y) - toOrigin.y};

            minDist = std::min(minDist, math::euclideanDistance(from, to));

            // Early breaking: this point is already closer to the other shape
            // than the current maximum, so it cannot raise it.
            if (!std::isnan(directedDist) && minDist < directedDist)
                break;
        }

        if (std::isnan(directedDist) || minDist > directedDist)
            directedDist = minDist;
    }

    // No distance has been evaluated.
    //
    // The only valid explanation is that every point of fromShape belongs
    // to the provided intersection. In that case the directed Hausdorff
    // distance is zero. Any other situation is unexpected and gives NaN.
    if (std::isnan(directedDist) && intersectionAB_ &&
        intersectionAB_->size() == fromShape.points().size())
    {
        return 0.f;
    }

    return directedDist;
}

float HausdorffDistance::distance() const
{
    if (std::isnan(distanceFromAToB_) || std::isnan(distanceFromBToA_))
        return std::numeric_limits<float>::quiet_NaN();

    return std::max(distanceFromAToB_, distanceFromBToA_);
}

float HausdorffDistance::centroidsDistance() const
{
    if (!shapeA_.isValid() || !shapeB_.isValid())
        return std::numeric_limits<float>::quiet_NaN();

    return math::euclideanDistance(shapeA_.centroid(), shapeB_.centroid());
}

} // namespace fluvel_ip
