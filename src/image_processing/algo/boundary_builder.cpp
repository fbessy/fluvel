// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "boundary_builder.hpp"

#include "fluvel_math.hpp"
#include "point_containers.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

namespace fluvel_ip
{

BoundaryBuilder::BoundaryBuilder(int phi_width, int phi_height, Contour& Lout_init,
                                 Contour& Lin_init)
    : grid_width_(phi_width)
    , grid_height_(phi_height)
    , Lout_init_(Lout_init)
    , Lin_init_(Lin_init)
{
    assert(phi_width >= 1);
    assert(phi_height >= 1);
}

void BoundaryBuilder::generate_rectangle_points(Point2D_i top_left, Point2D_i bottom_right,
                                                BoundaryOrientation orientation)
{
    auto* list1 = &Lout_init_;
    auto* list2 = &Lin_init_;

    if (orientation == BoundaryOrientation::Reversed)
    {
        std::swap(list1, list2);
    }

    generate_rectangle_points(top_left.x, top_left.y, bottom_right.x, bottom_right.y, *list1,
                              *list2);
}

void BoundaryBuilder::generate_rectangle_points(Point2D_f top_left, Point2D_f bottom_right,
                                                BoundaryOrientation orientation)
{
    auto* list1 = &Lout_init_;
    auto* list2 = &Lin_init_;

    if (orientation == BoundaryOrientation::Reversed)
    {
        std::swap(list1, list2);
    }

    generate_rectangle_points(std::lround(top_left.x * grid_width_),
                              std::lround(top_left.y * grid_height_),
                              std::lround(bottom_right.x * grid_width_),
                              std::lround(bottom_right.y * grid_height_), *list1, *list2);
}

void BoundaryBuilder::generate_rectangle_points(int x1, int y1, int x2, int y2, Contour& list_out,
                                                Contour& list_in)
{
    if (x1 > x2)
    {
        std::swap(x1, x2);
    }
    if (y1 > y2)
    {
        std::swap(y1, y2);
    }

    if (x1 != x2 && y1 != y2)
    {
        generate_rectangle_points_for_one_list(list_in, x1, y1, x2, y2);

#ifdef ALGO_8_CONNEXITY
        generate_rectangle_points_for_one_list(list_out, x1 - 1, y1 - 1, x2 + 1, y2 + 1);
#else
        for (int x = x1; x <= x2; ++x)
        {
            if (x >= 0 && x < grid_width_)
            {
                if (y1 - 1 >= 0 && y1 - 1 < grid_height_)
                    list_out.emplace_back(x, y1 - 1);

                if (y2 + 1 >= 0 && y2 + 1 < grid_height_)
                    list_out.emplace_back(x, y2 + 1);
            }
        }

        for (int y = y1; y <= y2; ++y)
        {
            if (y >= 0 && y < grid_height_)
            {
                if (x1 - 1 >= 0 && x1 - 1 < grid_width_)
                    list_out.emplace_back(x1 - 1, y);

                if (x2 + 1 >= 0 && x2 + 1 < grid_width_)
                    list_out.emplace_back(x2 + 1, y);
            }
        }
#endif
    }
}

void BoundaryBuilder::generate_rectangle_points_for_one_list(Contour& list_init, int x1, int y1,
                                                             int x2, int y2)
{
    for (int x = x1; x <= x2; ++x)
    {
        if (x >= 0 && x < grid_width_)
        {
            if (y1 >= 0 && y1 < grid_height_)
                list_init.emplace_back(x, y1);

            if (y2 >= 0 && y2 < grid_height_)
                list_init.emplace_back(x, y2);
        }
    }

    for (int y = y1 + 1; y < y2; ++y)
    {
        if (y >= 0 && y < grid_height_)
        {
            if (x1 >= 0 && x1 < grid_width_)
                list_init.emplace_back(x1, y);

            if (x2 >= 0 && x2 < grid_width_)
                list_init.emplace_back(x2, y);
        }
    }
}

void BoundaryBuilder::generate_ellipse_points(int width, int height, Point2D_i center,
                                              BoundaryOrientation orientation)
{
    auto* list1 = &Lout_init_;
    auto* list2 = &Lin_init_;

    if (orientation == BoundaryOrientation::Reversed)
    {
        std::swap(list1, list2);
    }

    const int a = width / 2;
    const int b = height / 2;

    generate_ellipse_points(center.x, center.y, a, b, *list1, *list2);
}

void BoundaryBuilder::generate_ellipse_points(float width_ratio, float height_ratio,
                                              Point2D_f center, BoundaryOrientation orientation)
{
    auto* list1 = &Lout_init_;
    auto* list2 = &Lin_init_;

    if (orientation == BoundaryOrientation::Reversed)
    {
        std::swap(list1, list2);
    }

    const float cx = (center.x + 0.5f) * grid_width_;
    const float cy = (center.y + 0.5f) * grid_height_;

    const int x0 = std::lround(cx);
    const int y0 = std::lround(cy);

    const int a = std::lround((width_ratio / 2.f) * grid_width_);
    const int b = std::lround((height_ratio / 2.f) * grid_height_);

    generate_ellipse_points(x0, y0, a, b, *list1, *list2);
}

void BoundaryBuilder::generate_ellipse_points(int x0, int y0, int a, int b, Contour& list_out,
                                              Contour& list_in)
{
    build_ellipse_midpoint_connected(x0, y0, a, b, list_out);
    build_inner_contiguous(x0, y0, list_out, list_in);
}

void BoundaryBuilder::build_ellipse_midpoint_connected(int x0, int y0, int a, int b,
                                                       Contour& list_out)
{
    int x = 0;
    int y = b;

    // Carrés
    int64_t a2 = static_cast<int64_t>(a * a);
    int64_t b2 = static_cast<int64_t>(b * b);

    int64_t dx = 0;
    int64_t dy = 2 * a2 * y;

    // Paramètre de décision région 1
    int64_t d1 = b2 - (a2 * b) + (a2 / 4);

    int prev_x = x;
    int prev_y = y;

    // -------- Région 1 --------
    while (dx < dy)
    {
        // Ajout du point principal
        add_4_points_in_ellipse(list_out, x, y, x0, y0);

        // ---- Connexité 8 ----
        int dxp = x - prev_x;
        int dyp = y - prev_y;

        if (abs(dxp) == 1 && abs(dyp) == 1)
        {
            // pixel correcteur (choix simple et sûr)
            add_4_points_in_ellipse(list_out, prev_x, y, x0, y0);
        }

        prev_x = x;
        prev_y = y;

        ++x;
        dx += (2 * b2);

        if (d1 < 0)
        {
            d1 += (dx + b2);
        }
        else
        {
            --y;
            dy -= (2 * a2);
            d1 += (dx - dy + b2);
        }
    }

    // Paramètre région 2
    const int64_t x_shift = static_cast<int64_t>(x) * 2 + 1; // 2*(x + 0.5)
    const int64_t y_shift = static_cast<int64_t>(y) - 1;

    int64_t d2 = (b2 * x_shift * x_shift / 4) + (a2 * y_shift * y_shift) - (a2 * b2);

    // -------- Région 2 --------
    while (y >= 0)
    {
        add_4_points_in_ellipse(list_out, x, y, x0, y0);

        // ---- Connexité 8 ----
        int dxp = x - prev_x;
        int dyp = y - prev_y;

        if (std::abs(dxp) == 1 && std::abs(dyp) == 1)
        {
            add_4_points_in_ellipse(list_out, x, prev_y, x0, y0);
        }

        prev_x = x;
        prev_y = y;

        --y;
        dy -= (2 * a2);

        if (d2 > 0)
        {
            d2 += (a2 - dy);
        }
        else
        {
            ++x;
            dx += (2 * b2);
            d2 += (dx - dy + a2);
        }
    }
}

void BoundaryBuilder::build_inner_contiguous(int x0, int y0, const Contour& l_out, Contour& l_in)
{
    PointSet seen;
    seen.reserve(l_out.size());

    for (const auto& p : l_out)
    {
        const int sx = math::sign(x0 - p.x());
        const int sy = math::sign(y0 - p.y());

        const Point2D_i pi{p.x() + sx, p.y() + sy};

        if (!inside_grid(pi))
            continue;

        if (seen.insert(pi).second)
            l_in.emplace_back(pi);
    }
}

void BoundaryBuilder::check_duplicates(const Contour& contour)
{
    PointSet seen;
    seen.reserve(contour.size());

    for (const auto& p : contour)
    {
        const Point2D_i point{p.x(), p.y()};

        if (!seen.insert(point).second)
        {
            std::cerr << " pos=(" << point.x << "," << point.y << ")\n";
            std::cerr.flush();
        }
    }
}

} // namespace fluvel_ip
