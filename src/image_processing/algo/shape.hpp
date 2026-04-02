// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "point.hpp"
#include <vector>

namespace fluvel_ip
{

class Shape
{
public:
    //! Constructor.
    Shape();

    //! Increase the capacity of the vector points.
    void reserve(size_t elem_alloc_size);

    //! Clear all the points of the shape.
    void clear();

    //! Push back a point into the shape.
    void pushBack(int x, int y);

    //! Push back a point into the shape.
    void pushBack(const Point2D_i& p);

    //! Push back a point into the shape.
    void pushBack(Point2D_i&& p);

    //! Swap the shape *this with an other shape in constant time, i.e. O(1) complexity.
    void swap(Shape& other) noexcept;

    //! Shuffles points of the shape.
    void shufflePoints();

    //! Calculates the centroid of the shape in variables #centroid_x and #centroid_y.
    void calculateCentroid();

    //! Returns true if the shape is ready for the hausdorff distance computation.
    bool isValid() const;

    //! Gets the vector of points.
    const std::vector<Point2D_i>& points() const
    {
        return points_;
    }

    //! Gets the centroid of the shape.
    const Point2D_f& centroid() const
    {
        return centroid_;
    }

    //! Gets grid diagonal.
    static float gridDiagonal(int gridWidth, int gridHeight);

private:
    std::vector<Point2D_i> points_;

    //! Position of the shape's centroid.
    Point2D_f centroid_;
};

} // namespace fluvel_ip
