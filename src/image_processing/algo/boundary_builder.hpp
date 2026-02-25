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

#ifndef BOUNDARY_BUILDER_HPP
#define BOUNDARY_BUILDER_HPP

#include "contour_data.hpp"
#include "point.hpp"
#include <vector>

namespace ofeli_ip
{

enum class BoundaryOrientation
{
    Normal,
    Reversed
};

class BoundaryBuilder
{
public:
    BoundaryBuilder(int phi_width1, int phi_height1, Contour& l_out_init1, Contour& l_in_init1);

    //! Gets rectangle points with boundary box (x1,y1) and (x2,y2).
    void generate_rectangle_points(Point2D_i top_left, Point2D_i bottom_right,
                                   BoundaryOrientation orientation = BoundaryOrientation::Normal);

    //! Gets rectangle points with normalized boundary box (x1,y1) and (x2,y2).
    void generate_rectangle_points(Point2D_f top_left, Point2D_f bottom_right,
                                   BoundaryOrientation orientation = BoundaryOrientation::Normal);

    //! Gets ellipse points with center point and width and height.
    void generate_ellipse_points(int width, int height, Point2D_i center,
                                 BoundaryOrientation orientation = BoundaryOrientation::Normal);

    //! Gets ellipse points with normalized center point and width and height.
    void generate_ellipse_points(float width_ratio, float height_ratio,
                                 Point2D_f center = {}, // value-initialized to (0.f, 0.f)
                                 BoundaryOrientation orientation = BoundaryOrientation::Normal);

private:
    void generate_rectangle_points(int x1, int y1, int x2, int y2, Contour& list_out,
                                   Contour& list_in);

    void generate_rectangle_points_for_one_list(Contour& list_init, int x1, int y1, int x2, int y2);

    //! Gets ellipse points with center point and width and height.
    void generate_ellipse_points(int x0, int y0, int a, int b, Contour& list_out, Contour& list_in);

    void build_ellipse_midpoint_connected(int x0, int y0, int a, int b, Contour& list_out);

    void build_inner_contiguous(int x0, int y0, const Contour& l_out, Contour& l_in);

    void check_duplicates(const Contour& contour);

    void add_4_points_in_ellipse(Contour& list_init, int x, int y, int x0, int y0);

    void add_point_unique(Contour& out, int x, int y);

    bool inside_grid(int x, int y) const;
    inline bool inside_grid(const Point2D_i& p) const;

    int grid_width_;
    int grid_height_;

    Contour& Lout_init_;
    Contour& Lin_init_;
};

inline bool BoundaryBuilder::inside_grid(int x, int y) const
{
    return x >= 0 && x < grid_width_ && y >= 0 && y < grid_height_;
}

inline bool BoundaryBuilder::inside_grid(const Point2D_i& p) const
{
    return inside_grid(p.x, p.y);
}

inline void BoundaryBuilder::add_4_points_in_ellipse(Contour& list, int x, int y, int x0, int y0)
{
    if (inside_grid(x0 - x, y0 - y))
        add_point_unique(list, x0 - x, y0 - y);

    if (inside_grid(x0 - x, y0 + y))
        add_point_unique(list, x0 - x, y0 + y);

    if (inside_grid(x0 + x, y0 - y))
        add_point_unique(list, x0 + x, y0 - y);

    if (inside_grid(x0 + x, y0 + y))
        add_point_unique(list, x0 + x, y0 + y);
}

inline void BoundaryBuilder::add_point_unique(Contour& out, int x, int y)
{
    const ContourPoint p{x, y};

    constexpr size_t WINDOW = 4;
    const size_t n = out.size();

    const size_t begin = (n > WINDOW) ? (n - WINDOW) : 0;

    for (size_t i = begin; i < n; ++i)
    {
        if (out[i] == p)
            return;
    }

    out.push_back(p);
}

} // namespace ofeli_ip

#endif // BOUNDARY_BUILDER_HPP
