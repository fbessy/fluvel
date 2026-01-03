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

#ifndef HAUSDORFF_DISTANCE_HPP
#define HAUSDORFF_DISTANCE_HPP

#include <vector>
#include <unordered_set>

#include "shape.hpp"
#include "point_hash.hpp"

namespace ofeli_ip
{

class HausdorffDistance
{

public :

    //! Constructor.
    HausdorffDistance(Shape& shape_a1,
                      Shape& shape_b1,
                      const std::unordered_set<Point_i>& intersection1 = std::unordered_set<Point_i>());

    //! Gets the hausdorff distance between #shape_a and #shape_b.
    float get_distance() const;

    //! Gets the modified hausdorff distance between #shape_a and #shape_b.
    //! Note : the result of the modified hausdorff distance is not exact (and higher than the exact value)
    //! if the optimization EARLY_BREAKING is applied.
    float get_modified() const;

    //! Gets the hausdorff quantile between #shape_a and #shape_b.
    float get_hausdorff_quantile(int hundredth);

    //! Gets the centroids distance, i.e. the gap between the #shape_a 's centroid and the #shape_b 's centroid.
    float get_centroids_distance() const;

private:

    //! Computes the hausdorff distance (and minimum distances in same time).
    void compute();

    //! Computes the directed or relative hausdorff distance
    //! (and minimum distances in same time, in one direction).
    void compute_directed_hd(const Shape& shape_1,
                             const Shape& shape_2,
                             float& directed_hd,
                             std::vector<float>& directed_min_dists);

    //! Gives the square of a value.
    template <typename T>
    static T square(const T& value);

    //! Mean or average of the directed/relative minimum distances.
    static float calculate_mean(const std::vector<float>& min_dists);

    //! Gets the directed/relative minimum Hausdforff quantile.
    static float get_hausdorff_quantile(const std::vector<float>& min_dists,
                                        int hundredth);

    //! Shape a defined by its points offsets.
    Shape& shape_a;

    //! Shape b defined by its points offsets.
    Shape& shape_b;

    //! Optional intersection with common points between #shape_a and #shape_b.
    const std::unordered_set<Point_i>& intersection_a_b;

    //! Directed or relative hausdorff distance from #shape_a (outer loop) to #shape_b (inner loop).
    float hd_a_to_b;

    //! Directed or relative hausdorff distance from #shape_b (outer loop) to #shape_a (inner loop).
    float hd_b_to_a;

    //! Minimum distances computed in the direction from #shape_a (outer loop) to #shape_b (inner loop).
    std::vector<float> min_dists_a_to_b;

    //! Minimum distances computed in the direction from #shape_b (outer loop) to #shape_a (inner loop).
    std::vector<float> min_dists_b_to_a;

    //! Position in x of the centroid of #shape_a.
    int centroid_a_x;
    //! Position in y of the centroid of #shape_a.
    int centroid_a_y;

    //! Position in x of the centroid of #shape_b.
    int centroid_b_x;
    //! Position in y of the centroid of #shape_b.
    int centroid_b_y;

    //! Boolean to know if min_dists are already sorted.
    bool is_sorted;
};

template <typename T>
inline T HausdorffDistance::square(const T& value)
{
    return value*value;
}

}

#endif // HAUSDORFF_DISTANCE_HPP
