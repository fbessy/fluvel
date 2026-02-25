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
#include "ofeli_math.hpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <random>

namespace ofeli_ip
{

Shape::Shape()
    : centroid_(std::numeric_limits<float>::min(), std::numeric_limits<float>::min())
{
}

void Shape::reserve(size_t elem_alloc_size)
{
    points_.reserve(elem_alloc_size);
}

void Shape::clear()
{
    points_.clear();
}

void Shape::push_back(int x, int y)
{
    points_.emplace_back(x, y);
}

void Shape::push_back(const Point2D_i& p)
{
    points_.push_back(p);
}

void Shape::push_back(Point2D_i&& p)
{
    points_.emplace_back(p);
}

void Shape::swap(Shape& other) noexcept
{
    this->points_.swap(other.points_);

    std::swap(this->centroid_.x, other.centroid_.x);
    std::swap(this->centroid_.y, other.centroid_.y);
}

void Shape::shuffle_points()
{
    auto rng = std::default_random_engine{};
    std::ranges::shuffle(points_, rng);
}

void Shape::calculate_centroid()
{
    centroid_.x = std::numeric_limits<float>::min();
    centroid_.y = std::numeric_limits<float>::min();

    if (points_.size() >= 1)
    {
        Point2D_i sum(0, 0);

        for (const auto& p : points_)
        {
            sum += p;
        }

        centroid_.x = float(sum.x) / float(points_.size());
        centroid_.y = float(sum.y) / float(points_.size());
    }
}

bool Shape::is_valid() const
{
    return (!points_.empty() && centroid_.x >= 0.f && centroid_.y >= 0.f);
}

float Shape::get_grid_diagonal(int grid_width, int grid_height)
{
    assert(grid_width >= 0);
    assert(grid_height >= 0);

    Point2D_i top_left{0, 0};
    Point2D_i bottom_right{grid_width - 1, grid_height - 1};

    return math::euclidean_distance(top_left, bottom_right);
}

} // namespace ofeli_ip
