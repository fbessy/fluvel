// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "point_containers.hpp"
#include "shape.hpp"

#include <vector>

namespace fluvel_ip
{

class HausdorffDistance
{
public:
    //! Constructor.
    HausdorffDistance(const Shape& shapeA, const Shape& shapeB,
                      const PointSet* intersectionAB = nullptr);

    //! Gets the hausdorff distance between #shape_a and #shape_b.
    float distance() const;

    //! Gets the modified hausdorff distance between #shape_a and #shape_b.
    //! Note : the result of the modified hausdorff distance is not exact (and higher than the
    //! exact value) if the optimization EARLY_BREAKING is applied.
    float modifiedDistance() const;

    //! Gets the hausdorff quantile between #shape_a and #shape_b.
    float hausdorffQuantile(int hundredth);

    //! Gets the centroids distance, i.e. the gap between the #shape_a 's centroid and the
    //! #shape_b 's centroid.
    float centroidsDistance() const;

private:
    //! Computes the hausdorff distance (and minimum distances in same time).
    void compute();

    //! Computes the directed or relative hausdorff distance
    //! (and minimum distances in same time, in one direction).
    void computeDirectedHausdorff(const Shape& fromShape, const Shape& toShape,
                                  float& directedDistance, std::vector<float>& minDistances);

    //! Mean or average of the directed/relative minimum distances.
    static float calculateMean(const std::vector<float>& minDists);

    //! Gets the directed/relative minimum Hausdforff quantile.
    static float hausdorffQuantile(const std::vector<float>& minDists, int hundredth);

    //! Shape a defined by its points offsets.
    Shape shapeA_;

    //! Shape b defined by its points offsets.
    Shape shapeB_;

    //! Optionnal intersection (points in common between A and B).
    const PointSet* intersectionAB_{nullptr};

    //! Directed or relative hausdorff distance from #shape_a (outer loop) to #shape_b (inner
    //! loop).
    float distanceFromAToB_{0.f};

    //! Directed or relative hausdorff distance from #shape_b (outer loop) to #shape_a (inner
    //! loop).
    float distanceFromBToA_{0.f};

    //! Minimum distances computed in the direction from #shape_a (outer loop) to #shape_b
    //! (inner loop).
    std::vector<float> minDistancesFromAToB_;

    //! Minimum distances computed in the direction from #shape_b (outer loop) to #shape_a
    //! (inner loop).
    std::vector<float> minDistancesFromBToA_;

    //! Boolean to know if minDists are already sorted.
    bool isSorted_{false};
};

} // namespace fluvel_ip
