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

#include "matrix.hpp"

namespace ofeli_ip
{

enum class PhiValue : int8_t
{
    InsideRegion      = -3,
    InteriorBoundary  = -1,
    ExteriorBoundary  = 1,
    OutsideRegion     = 3
};

using DiscreteLevelSet = Matrix2D<PhiValue>;

enum class SpeedValue : int8_t
{
    GoInward  = -1,
    NoMove    =  0,
    GoOutward =  1
};

class ContourPoint
{
public:
    ContourPoint(int offset1, int x1): offset_(offset1), x_(x1)
    {}

    int offset() const { return offset_; }
    int x() const { return x_; }
    SpeedValue speed() const { return speed_; }
    void set_speed(SpeedValue speed) { speed_ = speed; }

private:
    int offset_;
    int x_; // in order to check fastly neighborhood existence (border cases to handle)

    //! Internal speed Fint or external speed Fd to evolve the active contour in one direction locally.
    SpeedValue speed_;
};

using ContourList = std::vector<ContourPoint>;

class ContourData
{

public :

    //! Constructor to initialize the contour with one ellipse.
    ContourData(int phi_width, int phi_height);

    //! Constructor to initialize the contour from a grayscale image data buffer of the levet-set function #phi.
    ContourData(const unsigned char* phi_grayscale_img_data,
                int phi_width, int phi_height);

    //! Constructor to initialize the contour with the both neighbouring boundaries lists of #l_out and #l_in.
    ContourData(const ContourList& l_out,
                const ContourList& l_in,
                int phi_width, int phi_height);

    //! Copy constructor.
    ContourData(const ContourData& contour);

    //! Move constructor.
    ContourData(ContourData&& contour) noexcept;

    //! Checks if a given point is redundant to define a boundary, i.e. if no neighbors have a different phi value sign comparing to the given point.
    bool is_redundant(const ContourPoint& point) const;

    //! Checks lists.
    bool check_lists();

    //! Allocate lists.
    void allocate_lists();

    //! Wrapper to use directly with offset lists without the need to get the variable #phi.
    Point2D_i coord(int offet) const
    {
        return phi_.coord( offet );
    }

    //! Getter function for the discrete level-set function #phi.
    DiscreteLevelSet& phi() { return phi_; }
    const DiscreteLevelSet& phi() const { return phi_; }
    //! Getter function for the exterior boundary #l_out.
    ContourList& l_out() { return l_out_; }
    const ContourList& l_out() const { return l_out_; }
    //! Getter function for the interior boundary #l_in.
    ContourList& l_in() { return l_in_; }
    const ContourList& l_in() const { return l_in_; }

    size_t preallocation_size() const { return preallocation_size_; }

private :

    //! Initializes the contour *this with one ellipse. It is performed when the simplest constructor is called or when one or both boundary lists is/are empty.
    void initialize_with_one_ellipse();

    //! Defines the #phi level-set function from the boundary lists #l_out and #l_in.
    void define_phi_with_boundary();

    //! Performs a flood fill algorithm for the method #define_phi_with_boundary().
    void do_flood_fill(int offset_seed,
                       PhiValue target_value,
                       PhiValue replacement_value);

    //! Checks phi dimension.
    static bool is_ok_phi_dimension(int dimension);

    //! Discrete level-set function with only 4 PhiValue possible.
    DiscreteLevelSet phi_;

    //! List of points representing the exterior boundary (called Lout in the reference paper).
    ContourList l_out_;
    //! List of points representing the interior boundary (called Lin in the reference paper).
    ContourList l_in_;

    //! Preallocation size for each list.
    size_t preallocation_size_;
};

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

}

}

#endif // CONTOUR_DATA_HPP
