// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

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
