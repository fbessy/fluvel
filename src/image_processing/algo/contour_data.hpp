/****************************************************************************
**
** Copyright (C) 2010-2025 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.F
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

#ifndef CONTOUR_DATA_HPP
#define CONTOUR_DATA_HPP

#include <cstdint>

#include <vector>

#include "grid2d.hpp"
#include "image_span.hpp"

namespace ofeli_ip
{

//! Pixel connectivity of the neighborhood used by the algorithm (4- or 8-connected).
enum class Connectivity
{
    Four,
    Eight
};

enum class PhiValue : int8_t
{
    InsideRegion = -3,
    InteriorBoundary = -1,
    ExteriorBoundary = 1,
    OutsideRegion = 3
};

using DiscreteLevelSet = Grid2D<PhiValue>;

enum class SpeedValue : int8_t
{
    GoInward = -1,
    NoMove = 0,
    GoOutward = 1
};

class ContourPoint
{
public:
    ContourPoint(int x, int y)
        : x_(x)
        , y_(y)
        , speed_(SpeedValue::NoMove)
    {
    }
    ContourPoint(const Point2D_i& p)
        : x_(p.x)
        , y_(p.y)
        , speed_(SpeedValue::NoMove)
    {
    }

    Point2D_i pos() const noexcept
    {
        return {x_, y_};
    }
    int x() const noexcept
    {
        return x_;
    }
    int y() const noexcept
    {
        return y_;
    }

    bool operator==(const ContourPoint& other) const noexcept
    {
        return x_ == other.x_ && y_ == other.y_;
    }

    bool operator!=(const ContourPoint& other) const noexcept
    {
        return !(*this == other);
    }

private:
    int x_;
    int y_;

    //! Pending sign speed of the algorithm to drive the contour
    SpeedValue speed_;

    friend class ActiveContour;
    friend class RegionAc;
    friend class RegionColorAc;
    friend class EdgeAc;
};

using Contour = std::vector<ContourPoint>;
using ExportedContour = std::vector<Point2D_i>;

class ContourData
{
public:
    //! Constructor to initialize the contour with one ellipse.
    ContourData(int phi_width, int phi_height, Connectivity connectivity = Connectivity::Four);

    //! Constructor to initialize the contour from a grayscale image of the levet-set function
    //! #phi.
    ContourData(ImageSpan grayscale_phi, Connectivity connectivity = Connectivity::Four);

    //! Constructor to initialize the contour with the both neighbouring boundaries lists of
    //! #l_out and #l_in.
    ContourData(const Contour& l_out, const Contour& l_in, int phi_width, int phi_height,
                Connectivity connectivity = Connectivity::Four);

    //! Copy constructor.
    ContourData(const ContourData& contour);

    //! Move constructor.
    ContourData(ContourData&& contour) noexcept;

    //! Wrapper to use directly with offset lists without the need to get the variable #phi.
    Point2D_i coord(size_t offet) const
    {
        return phi_.coord(offet);
    }

    //! Export the boundary list l_out_ as a copied geometric representation.
    ExportedContour export_l_out() const
    {
        return export_contour(l_out_);
    }

    //! Export the boundary list l_in_ as a copied geometric representation.
    ExportedContour export_l_in() const
    {
        return export_contour(l_in_);
    }

    //! Getter function for the discrete level-set function #phi.
    DiscreteLevelSet& phi()
    {
        return phi_;
    }
    const DiscreteLevelSet& phi() const
    {
        return phi_;
    }
    //! Getter function for the exterior boundary #l_out.
    Contour& l_out()
    {
        return l_out_;
    }
    const Contour& l_out() const
    {
        return l_out_;
    }
    //! Getter function for the interior boundary #l_in.
    Contour& l_in()
    {
        return l_in_;
    }
    const Contour& l_in() const
    {
        return l_in_;
    }

    bool empty() const
    {
        return l_out_.empty() || l_in_.empty();
    }

    bool is_valid() const;

    Connectivity connectivity() const
    {
        return connectivity_;
    }

private:
    //! Initializes the contour *this with one ellipse. It is performed when the simplest
    //! constructor is called or when one or both boundary lists is/are empty.
    void define_from_ellipse();

    //! Eliminates redundant points to maintain a contiguous boundary.
    void eliminate_redundant_points(Contour& boundary, PhiValue region_value);

    //! To use define_from_ellipse and eliminate_redundant_points
    //! in ActiveContour.
    friend class ActiveContour;

    //! Checks if a given point is redundant to define a boundary, i.e. if no neighbors have a
    //! different phi value sign comparing to the given point.
    bool is_redundant(const ContourPoint& point) const;

    //! Allocate lists.
    void allocate_lists();

    //! Defines the boundary lists and phi from a binary phi.
    void define_lists_and_phi_from_binary_phi();

    //! Defines the #phi level-set function from the boundary lists #l_out and #l_in.
    void define_phi_from_lists();

    //! Performs a flood fill algorithm for the method #define_phi_from_lists().
    void flood_fill(const Point2D_i& seed, PhiValue target_value, PhiValue replacement_value);

    //! Eliminates redundant points for the both lists to maintain a contiguous boundary.
    void eliminate_redundant_points_if_needed();

    //! Export a boundary list as a copied geometric representation.
    std::vector<Point2D_i> export_contour(const Contour& boundary) const;

    template <typename T> static bool has_duplicates(const std::vector<T>& v);

    //! Discrete level-set function with only 4 PhiValue possible.
    DiscreteLevelSet phi_;

    //! List of points representing the exterior boundary (called Lout in the reference paper).
    Contour l_out_;
    //! List of points representing the interior boundary (called Lin in the reference paper).
    Contour l_in_;

    //! Pixel connectivity of the neighborhood to define ContourData (4- or 8-connected).
    const Connectivity connectivity_;
};

template <typename T> bool ContourData::has_duplicates(const std::vector<T>& v)
{
    for (std::size_t i = 0; i < v.size(); ++i)
        for (std::size_t j = i + 1; j < v.size(); ++j)
            if (v[i] == v[j])
                return true;
    return false;
}

namespace phi_value
{

constexpr int phiSign(PhiValue v);
constexpr bool isInside(PhiValue v);
constexpr bool isOutside(PhiValue v);
constexpr bool differentSide(PhiValue a, PhiValue b);

/// Discrete sign of level-set function.
/// Returns -1 for interior side, +1 for exterior side.
/// Note: zero level-set is conceptual and never stored.
constexpr int phiSign(PhiValue v)
{
    switch (v)
    {
        case PhiValue::InsideRegion:
        case PhiValue::InteriorBoundary:
            return -1;

        case PhiValue::ExteriorBoundary:
        case PhiValue::OutsideRegion:
            return 1;
    }
    return 0; // unreachable
}

constexpr bool isInside(PhiValue v)
{
    return phiSign(v) < 0;
}

constexpr bool isOutside(PhiValue v)
{
    return !isInside(v);
}

constexpr bool differentSide(PhiValue a, PhiValue b)
{
    return phiSign(a) * phiSign(b) < 0;
}

} // namespace phi_value

} // namespace ofeli_ip

#endif // CONTOUR_DATA_HPP
