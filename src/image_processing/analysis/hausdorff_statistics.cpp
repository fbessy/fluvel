// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "hausdorff_statistics.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

#include "fluvel_math.hpp"

namespace fluvel_ip
{

HausdorffStatistics::HausdorffStatistics(const Shape& shapeA, const Shape& shapeB,
                                         const PointSet* intersectionAB, HausdorffOptions options)
    : intersectionAB_(options.alignCentroids ? nullptr : intersectionAB)
    , options_(options)
{
    if (!shapeA.isValid() || !shapeB.isValid())
        return;

    minimaAToB_ = sortedMinima(shapeA, shapeB);
    minimaBToA_ = sortedMinima(shapeB, shapeA);
}

std::vector<float> HausdorffStatistics::sortedMinima(const Shape& fromShape,
                                                     const Shape& toShape) const
{
    const Point2D_f fromOrigin =
        options_.alignCentroids ? fromShape.centroid() : Point2D_f{0.f, 0.f};
    const Point2D_f toOrigin = options_.alignCentroids ? toShape.centroid() : Point2D_f{0.f, 0.f};

    std::vector<float> minima;
    minima.reserve(fromShape.points().size());

    for (const auto& fromPoint : fromShape.points())
    {
        // With absolute coordinates, a shared point is exactly at distance 0.
        if (intersectionAB_ && intersectionAB_->contains(fromPoint))
        {
            minima.push_back(0.f);
            continue;
        }

        const Point2D_f from{float(fromPoint.x) - fromOrigin.x,
                             float(fromPoint.y) - fromOrigin.y};

        float minDist = std::numeric_limits<float>::max();

        for (const auto& toPoint : toShape.points())
        {
            const Point2D_f to{float(toPoint.x) - toOrigin.x, float(toPoint.y) - toOrigin.y};

            minDist = std::min(minDist, math::euclideanDistance(from, to));

            if (minDist == 0.f)
                break;
        }

        minima.push_back(minDist);
    }

    std::ranges::sort(minima);

    return minima;
}

float HausdorffStatistics::modifiedDistance() const
{
    const float meanAToB = mean(minimaAToB_);
    const float meanBToA = mean(minimaBToA_);

    if (std::isnan(meanAToB) || std::isnan(meanBToA))
        return std::numeric_limits<float>::quiet_NaN();

    return std::max(meanAToB, meanBToA);
}

float HausdorffStatistics::quantileDistance(int hundredth) const
{
    const float quantileAToB = quantile(minimaAToB_, hundredth);
    const float quantileBToA = quantile(minimaBToA_, hundredth);

    if (std::isnan(quantileAToB) || std::isnan(quantileBToA))
        return std::numeric_limits<float>::quiet_NaN();

    return std::max(quantileAToB, quantileBToA);
}

float HausdorffStatistics::mean(const std::vector<float>& values)
{
    if (values.empty())
        return std::numeric_limits<float>::quiet_NaN();

    // double accumulator: thousands of float values
    const double sum = std::accumulate(values.begin(), values.end(), 0.0);

    return static_cast<float>(sum / static_cast<double>(values.size()));
}

float HausdorffStatistics::quantile(const std::vector<float>& sortedValues, int hundredth)
{
    if (sortedValues.empty())
        return std::numeric_limits<float>::quiet_NaN();

    const int h = std::clamp(hundredth, 0, 100);

    const int idx = ((int(sortedValues.size()) - 1) * h) / 100;

    return sortedValues[idx];
}

} // namespace fluvel_ip
