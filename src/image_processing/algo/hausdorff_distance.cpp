/****************************************************************************
**
** Copyright (C) 2010-2025 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.
**
** http://www.cecill.info/licences/Licence_CeCILL_V2-en.html
** You may use this file under the terms of the CeCILL license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Fabien Bessy and its Subsidiary(-ies) nor the
**     names of its contributors may be used to endorse or promote products
**     derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

#include "hausdorff_distance.hpp"

//#define NAIVE_ALGO

// According to
// "An Efficient Algorithm for Calculating
// the Exact Hausdorff Distance"
// Abdel Aziz Taha, Allan Hanbury, IEEE. PAMI, 2015

#ifndef NAIVE_ALGO

#define EARLY_BREAKING          // ×20 faster
#define RANDOM_SAMPLING         // ×40 faster with early breaking
#define INTERSECTION_EXCLUSION  // data dependant and proportional
                                // to the intersection part
#endif

#include <cmath>
#include <algorithm>
#include <numeric>
#include <cstddef>

#include "ofeli_math.hpp"

namespace ofeli_ip
{

constexpr size_t INITIAL_ARRAY_ALLOC_SIZE = 10000u;

//! Constructor. The third parameter is the intersection between shape a and b, i.e.
//! the points (offsets) in common. It's an optional parameter. It can speed up
//! the computation (proportionally of the intersection part) in case of the intersection is easily
//! to compute in constant time ( complexity in 0(1) ), with an image or a matrix.
HausdorffDistance::HausdorffDistance(Shape& shape_a1,
                                     Shape& shape_b1,
                                     const std::unordered_set<Point2D_i>& intersection1):
    shape_a(shape_a1),
    shape_b(shape_b1),
    intersection_a_b(intersection1),
    hd_a_to_b(std::numeric_limits<float>::max()),
    hd_b_to_a(std::numeric_limits<float>::max()),
    is_sorted(false)
{
    min_dists_a_to_b.reserve( INITIAL_ARRAY_ALLOC_SIZE );
    min_dists_b_to_a.reserve( INITIAL_ARRAY_ALLOC_SIZE );

    compute();
}

void HausdorffDistance::compute()
{
    if ( shape_a.is_valid() &&
         shape_b.is_valid() )
    {

#if defined(RANDOM_SAMPLING) && !defined(NAIVE_ALGO)
        shape_a.shuffle_points();
        shape_b.shuffle_points();
#endif

        // compute the directed or relative hausdorff distance in the both directions
        compute_directed_hd( shape_a, shape_b,
                             hd_a_to_b, min_dists_a_to_b );

        compute_directed_hd( shape_b, shape_a,
                             hd_b_to_a, min_dists_b_to_a );
    }
}

void HausdorffDistance::compute_directed_hd(const Shape& shape_1,
                                            const Shape& shape_2,
                                            float& directed_hd,
                                            std::vector<float>& directed_min_dists)
{
    Point2D_f relative_p1, relative_p2;

    float euclidean_dist, min_dist, sq;

    // initialization with a minimum value in order to maximize
    // the directed or relative hausdorff distance
    directed_hd = 0.f;

    // outer loop
    for( const auto& p1 : shape_1.get_points() )
    {
        // intersection_a_b is optional and can be empty by default,
        // cf constructor documentation.
        // It checks in constant time or O(1) complexity with the method contains
        // due to the stl unordered set container.

#if defined(INTERSECTION_EXCLUSION) && !defined(NAIVE_ALGO)
        if( !intersection_a_b.contains( p1 ) )
        {
#endif
            relative_p1.x = float(p1.x) - shape_1.get_centroid().x;
            relative_p1.y = float(p1.y) - shape_1.get_centroid().y;

            // initialization in order to minimize
            min_dist = std::numeric_limits<float>::max();

            // inner loop
            for( const auto& p2 : shape_2.get_points() )
            {
                relative_p2.x = float(p2.x) - shape_2.get_centroid().x;
                relative_p2.y = float(p2.y) - shape_2.get_centroid().y;

                // computes the square and the euclidean distance
                sq = ( math::square(relative_p2.x-relative_p1.x) +
                       math::square(relative_p2.y-relative_p1.y)   );

                if( sq > 0.f )
                {
                    euclidean_dist = std::sqrt( sq );
                }
                else if ( sq == 0.f )
                {
                    euclidean_dist = 0.f;
                }
                else
                {
                    euclidean_dist = std::numeric_limits<float>::max();
                }

                // it minimizes min_dist
                if( euclidean_dist < min_dist )
                {
                    min_dist = euclidean_dist;
                }

#if defined(EARLY_BREAKING) && !defined(NAIVE_ALGO)
                if ( min_dist < directed_hd )
                {
                    break;
                }
#endif
            }

            directed_min_dists.push_back( min_dist );

            // it maximizes the relative or directed hausdorff distance
            if( min_dist > directed_hd )
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
    if( directed_min_dists.empty()    &&
        !shape_1.get_points().empty() &&
        !shape_2.get_points().empty() )
    {
        directed_min_dists.push_back( 0.f );
    }
}

float HausdorffDistance::get_distance() const
{
    return std::max( hd_a_to_b,
                     hd_b_to_a );
}

float HausdorffDistance::get_modified() const
{
    return std::max( calculate_mean(min_dists_a_to_b),
                     calculate_mean(min_dists_b_to_a) );
}

float HausdorffDistance::get_hausdorff_quantile(int hundredth)
{
    if ( !is_sorted )
    {
        std::ranges::sort( min_dists_a_to_b );
        std::ranges::sort( min_dists_b_to_a );

        is_sorted = true;
    }

    return std::max( get_hausdorff_quantile( min_dists_a_to_b,
                                             hundredth ),
                     get_hausdorff_quantile( min_dists_b_to_a,
                                             hundredth ) );
}

float HausdorffDistance::calculate_mean(const std::vector<float>& min_dists)
{
    float mean = std::numeric_limits<float>::max();

    float sum = std::accumulate( min_dists.cbegin(),
                                 min_dists.cend(),
                                 0.f );

    if ( min_dists.size() >= 1 )
    {
        mean = ( sum / float( min_dists.size() ) );
    }

    return mean;
}

float HausdorffDistance::get_hausdorff_quantile(const std::vector<float>& min_dists,
                                                int hundredth)
{
    float quantile = std::numeric_limits<float>::max();
    int idx;

    if( hundredth < 0 )
    {
        // fist index
        idx = 0;
    }
    else if( hundredth > 100 )
    {
        // last index
        idx = (min_dists.size() - 1);
    }
    else
    {
        idx = ( (int(min_dists.size())-1) * hundredth / 100 );
    }

    if ( idx >= 0 &&
         idx < int(min_dists.size()) )
    {
        quantile = min_dists[ idx ];
    }

    return quantile;
}

float HausdorffDistance::get_centroids_distance() const
{
    float gap_dist = std::numeric_limits<float>::max();

    if( shape_a.is_valid() &&
        shape_b.is_valid() )
    {
        Point2D_f gap( shape_b.get_centroid().x - shape_a.get_centroid().x,
                     shape_b.get_centroid().y - shape_a.get_centroid().y );

        float sq_val = math::square( gap.x ) + math::square( gap.y );

        if( sq_val > 0.f )
        {
            gap_dist = std::sqrt( sq_val );
        }
        else if( sq_val == 0.f )
        {
            gap_dist = 0.f;
        }
    }

    return gap_dist;
}

}
