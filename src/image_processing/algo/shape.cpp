// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "shape.hpp"
#include "fluvel_math.hpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <random>

namespace fluvel_ip
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

void Shape::pushBack(int x, int y)
{
    points_.emplace_back(x, y);
}

void Shape::pushBack(const Point2D_i& p)
{
    points_.push_back(p);
}

void Shape::pushBack(Point2D_i&& p)
{
    points_.emplace_back(p);
}

void Shape::swap(Shape& other) noexcept
{
    this->points_.swap(other.points_);

    std::swap(this->centroid_.x, other.centroid_.x);
    std::swap(this->centroid_.y, other.centroid_.y);
}

void Shape::shufflePoints()
{
    auto rng = std::default_random_engine{};
    std::ranges::shuffle(points_, rng);
}

void Shape::calculateCentroid()
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

bool Shape::isValid() const
{
    return (!points_.empty() && centroid_.x >= 0.f && centroid_.y >= 0.f);
}

float Shape::gridDiagonal(int gridWidth, int gridHeight)
{
    assert(gridWidth >= 0);
    assert(gridHeight >= 0);

    Point2D_i topLeft{0, 0};
    Point2D_i bottomRight{gridWidth - 1, gridHeight - 1};

    return math::euclideanDistance(topLeft, bottomRight);
}

} // namespace fluvel_ip
