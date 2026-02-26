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

#include "ofeli_math.hpp"

namespace ofeli_ip
{

constexpr size_t kInitialArrayAllocSize = 10000u;

//! Constructor. The third parameter is the intersection between shape a and b, i.e.
//! the points (offsets) in common. It's an optional parameter. It can speed up
//! the computation (proportionally of the intersection part) in case of the intersection is
//! easily to compute in constant time ( complexity in 0(1) ), with an image or a matrix.
HausdorffDistance::HausdorffDistance(Shape& shape_a1, Shape& shape_b1,
                                     const PointSet& intersection1)
    : shape_a_(shape_a1)
    , shape_b_(shape_b1)
    , intersection_a_b_(intersection1)
{
    min_dists_a_to_b_.reserve(kInitialArrayAllocSize);
    min_dists_b_to_a_.reserve(kInitialArrayAllocSize);

    compute();
}

void HausdorffDistance::compute()
{
    if (shape_a_.is_valid() && shape_b_.is_valid())
    {
#if defined(RANDOM_SAMPLING) && !defined(NAIVE_ALGO)
        shape_a_.shuffle_points();
        shape_b_.shuffle_points();
#endif

        // compute the directed or relative hausdorff distance in the both directions
        compute_directed_hd(shape_a_, shape_b_, hd_a_to_b_, min_dists_a_to_b_);

        compute_directed_hd(shape_b_, shape_a_, hd_b_to_a_, min_dists_b_to_a_);
    }
}

void HausdorffDistance::compute_directed_hd(const Shape& shape_1, const Shape& shape_2,
                                            float& directed_hd,
                                            std::vector<float>& directed_min_dists)
{
    Point2D_f relative_p1, relative_p2;

    float dist, min_dist;

    // initialization with a minimum value in order to maximize
    // the directed or relative hausdorff distance
    directed_hd = 0.f;

    // outer loop
    for (const auto& p1 : shape_1.get_points())
    {
        // intersection_a_b is optional and can be empty by default,
        // cf constructor documentation.
        // It checks in constant time or O(1) complexity with the method contains
        // due to the stl unordered set container.

#if defined(INTERSECTION_EXCLUSION) && !defined(NAIVE_ALGO)
        if (!intersection_a_b_.contains(p1))
        {
#endif
            relative_p1.x = float(p1.x) - shape_1.get_centroid().x;
            relative_p1.y = float(p1.y) - shape_1.get_centroid().y;

            // initialization in order to minimize
            min_dist = std::numeric_limits<float>::max();

            // inner loop
            for (const auto& p2 : shape_2.get_points())
            {
                relative_p2.x = float(p2.x) - shape_2.get_centroid().x;
                relative_p2.y = float(p2.y) - shape_2.get_centroid().y;

                dist = math::euclidean_distance(relative_p1, relative_p2);

                // it minimizes min_dist
                if (dist < min_dist)
                {
                    min_dist = dist;
                }

#if defined(EARLY_BREAKING) && !defined(NAIVE_ALGO)
                if (min_dist < directed_hd)
                {
                    break;
                }
#endif
            }

            directed_min_dists.push_back(min_dist);

            // it maximizes the relative or directed hausdorff distance
            if (min_dist > directed_hd)
            {
                directed_hd = min_dist;
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
    if (directed_min_dists.empty() && !shape_1.get_points().empty() &&
        !shape_2.get_points().empty())
    {
        directed_min_dists.push_back(0.f);
    }
}

float HausdorffDistance::get_distance() const
{
    return std::max(hd_a_to_b_, hd_b_to_a_);
}

float HausdorffDistance::get_modified() const
{
    return std::max(calculate_mean(min_dists_a_to_b_), calculate_mean(min_dists_b_to_a_));
}

float HausdorffDistance::hausdorffQuantile(int hundredth)
{
    if (!is_sorted_)
    {
        std::ranges::sort(min_dists_a_to_b_);
        std::ranges::sort(min_dists_b_to_a_);

        is_sorted_ = true;
    }

    return std::max(hausdorffQuantile(min_dists_a_to_b_, hundredth),
                    hausdorffQuantile(min_dists_b_to_a_, hundredth));
}

float HausdorffDistance::calculate_mean(const std::vector<float>& min_dists)
{
    float mean = std::numeric_limits<float>::quiet_NaN();

    float sum = std::accumulate(min_dists.cbegin(), min_dists.cend(), 0.f);

    if (min_dists.size() >= 1)
    {
        mean = (sum / float(min_dists.size()));
    }

    return mean;
}

float HausdorffDistance::hausdorffQuantile(const std::vector<float>& min_dists, int hundredth)
{
    float quantile = std::numeric_limits<float>::quiet_NaN();
    int idx;

    if (hundredth < 0)
    {
        // fist index
        idx = 0;
    }
    else if (hundredth > 100)
    {
        // last index
        idx = (min_dists.size() - 1);
    }
    else
    {
        idx = ((int(min_dists.size()) - 1) * hundredth / 100);
    }

    if (idx >= 0 && idx < int(min_dists.size()))
    {
        quantile = min_dists[idx];
    }

    return quantile;
}

float HausdorffDistance::get_centroids_distance() const
{
    float gap_dist = std::numeric_limits<float>::quiet_NaN();

    if (shape_a_.is_valid() && shape_b_.is_valid())
    {
        gap_dist = math::euclidean_distance(shape_a_.get_centroid(), shape_b_.get_centroid());
    }

    return gap_dist;
}

} // namespace ofeli_ip
