// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "shape.hpp"
#include "fluvel_math.hpp"

#include <algorithm>
#include <cassert>

namespace fluvel_ip
{

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

void Shape::swap(Shape& other) noexcept
{
    std::swap(points_, other.points_);
    std::swap(centroid_, other.centroid_);
}

void Shape::shufflePoints()
{
    std::ranges::shuffle(points_, rng_);
}

void Shape::calculateCentroid()
{
    if (points_.empty())
    {
        centroid_.x = std::numeric_limits<float>::quiet_NaN();
        centroid_.y = std::numeric_limits<float>::quiet_NaN();

        return;
    }

    Point2D_i sum(0, 0);

    for (const auto& p : points_)
    {
        sum += p;
    }

    centroid_.x = static_cast<float>(sum.x) / static_cast<float>(points_.size());

    centroid_.y = static_cast<float>(sum.y) / static_cast<float>(points_.size());
}

bool Shape::isValid() const
{
    return !points_.empty() && !std::isnan(centroid_.x) && !std::isnan(centroid_.y);
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
