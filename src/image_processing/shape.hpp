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

#ifndef SHAPE_HPP
#define SHAPE_HPP

#include <vector>
#include "list_i.hpp"
#include "point.hpp"

namespace ofeli_ip
{

class Shape
{

public:

    //! Constructor.
    Shape();

    //! Constructor.
    Shape(int grid_width1, int grid_height1);

    //! Clear all the points of the shape.
    void clear();

    //! Push back an offset point into the shape.
    void push_back(int offset);

    //! Assign points from a List_i to points of Shape which is
    //! a std::vector and calculates the centroid.
    void transform(const List_i& list_i);

    //! Swap the shape *this with an other shape in constant time, i.e. O(1) complexity.
    void swap(Shape& other);

    //! Shuffles points of the shape.
    void shuffle_points();

    //! Calculates the centroid of the shape in variables #centroid_x and #centroid_y.
    void calculate_centroid();

    //! Returns true if the shape is ready for the hausdorff distance computation.
    bool is_valid() const;

    //! Gets the position (x,y) from an offset point.
    void get_position(int offset,
                      Point_i& point) const;

    //! Gets the vector of points.
    const std::vector<int>& get_points() const { return points; }

    //! Gets the centroid of the shape.
    const Point_f& get_centroid() const { return centroid; }

    //! Sets the grid size of the shape.
    void set_grid_size(int grid_width1,
                       int grid_height1)
    {
        grid_width  = grid_width1;
        grid_height = grid_height1;
    };

    //! Getter for #grid_width.
    int get_grid_width() const { return grid_width; }

    //! Getter for #grid_height.
    int get_grid_height() const { return grid_height; }

    //! Gets frid diagonal.
    float get_grid_diagonal() const;

private:

    //! Grid or matrix width.
    int grid_width;

    //! Grid or matrix height.
    int grid_height;

    //! Points are defined by offset in a grid.
    std::vector<int> points;

    //! Position of the shape's centroid.
    Point_f centroid;
};

}

#endif // SHAPE_HPP
