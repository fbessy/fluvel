// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "point_containers.hpp"
#include "shape.hpp"

#include <vector>

namespace ofeli_ip
{

class HausdorffDistance
{
public:
    //! Constructor.
    HausdorffDistance(Shape& shape_a1, Shape& shape_b1, const PointSet& intersection1 = PointSet());

    //! Gets the hausdorff distance between #shape_a and #shape_b.
    float get_distance() const;

    //! Gets the modified hausdorff distance between #shape_a and #shape_b.
    //! Note : the result of the modified hausdorff distance is not exact (and higher than the
    //! exact value) if the optimization EARLY_BREAKING is applied.
    float get_modified() const;

    //! Gets the hausdorff quantile between #shape_a and #shape_b.
    float hausdorffQuantile(int hundredth);

    //! Gets the centroids distance, i.e. the gap between the #shape_a 's centroid and the
    //! #shape_b 's centroid.
    float get_centroids_distance() const;

private:
    //! Computes the hausdorff distance (and minimum distances in same time).
    void compute();

    //! Computes the directed or relative hausdorff distance
    //! (and minimum distances in same time, in one direction).
    void compute_directed_hd(const Shape& shape_1, const Shape& shape_2, float& directed_hd,
                             std::vector<float>& directed_min_dists);

    //! Mean or average of the directed/relative minimum distances.
    static float calculate_mean(const std::vector<float>& min_dists);

    //! Gets the directed/relative minimum Hausdforff quantile.
    static float hausdorffQuantile(const std::vector<float>& min_dists, int hundredth);

    //! Shape a defined by its points offsets.
    Shape& shape_a_;

    //! Shape b defined by its points offsets.
    Shape& shape_b_;

    //! Optional intersection with common points between #shape_a and #shape_b.
    const PointSet& intersection_a_b_;

    //! Directed or relative hausdorff distance from #shape_a (outer loop) to #shape_b (inner
    //! loop).
    float hd_a_to_b_{0.f};

    //! Directed or relative hausdorff distance from #shape_b (outer loop) to #shape_a (inner
    //! loop).
    float hd_b_to_a_{0.f};

    //! Minimum distances computed in the direction from #shape_a (outer loop) to #shape_b
    //! (inner loop).
    std::vector<float> min_dists_a_to_b_;

    //! Minimum distances computed in the direction from #shape_b (outer loop) to #shape_a
    //! (inner loop).
    std::vector<float> min_dists_b_to_a_;

    //! Boolean to know if min_dists are already sorted.
    bool is_sorted_{false};
};

} // namespace ofeli_ip
