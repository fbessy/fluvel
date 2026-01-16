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

#include "shape.hpp"

#include <random>
#include <algorithm>
#include <limits>

namespace ofeli_ip
{

Shape::Shape(): Shape(100u)
{
}

Shape::Shape(size_t initial_array_alloc_size):
    centroid(std::numeric_limits<float>::min(),
             std::numeric_limits<float>::min())

{
    points.reserve( initial_array_alloc_size );
}

void Shape::clear()
{
    points.clear();
}

void Shape::push_back(int x, int y)
{
    points.emplace_back( x, y );
}

void Shape::push_back(const Point2D_i& p)
{
    points.push_back( p );
}

void Shape::push_back(Point2D_i&& p)
{
    points.emplace_back( std::move(p) );
}

void Shape::swap(Shape& other)
{
    this->points.swap(other.points);

    std::swap(this->centroid.x,  other.centroid.x);
    std::swap(this->centroid.y,  other.centroid.y);
}

void Shape::shuffle_points()
{
    auto rng = std::default_random_engine{};
    std::ranges::shuffle(points, rng);
}

void Shape::calculate_centroid()
{
    centroid.x = std::numeric_limits<float>::min();
    centroid.y = std::numeric_limits<float>::min();

    if( points.size() >= 1 )
    {
        Point2D_i sum( 0, 0 );

        for( const auto& p : points )
        {
            sum += p;
        }

        centroid.x = float( sum.x ) / float( points.size() );
        centroid.y = float( sum.y ) / float( points.size() );
    }
}

bool Shape::is_valid() const
{
    return ( !points.empty()         &&
              centroid.x      >= 0.f &&
              centroid.y      >= 0.f    );
}

float Shape::get_grid_diagonal(int grid_width, int grid_height)
{
    float square_sum = float( grid_width*grid_width + grid_height*grid_height );

    float result = std::numeric_limits<float>::max();

    if ( square_sum > 0.f )
    {
        result = std::sqrt( square_sum );
    }

    return result;
}

}
