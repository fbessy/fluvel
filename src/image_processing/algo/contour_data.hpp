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

enum class SpeedValue : int8_t
{
    GoInward  = -1,
    NoMove    =  0,
    GoOutward =  1
};

class ContourPoint
{
public:
    ContourPoint(int offset1, int x1): offset(offset1), x(x1)
    {}

    int get_offset() const { return offset; }
    int get_x() const { return x; }
    SpeedValue get_speed() const { return speed; }
    void set_speed(SpeedValue speed1) { speed = speed1; }

private:
    int offset;
    int x; // in order to check fastly neighborhood existence (border cases to handle)

    //! Internal speed Fint or external speed Fd to evolve the active contour in one direction locally.
    SpeedValue speed;
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
    ContourData(const ContourList& l_out1,
                const ContourList& l_in1,
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
    Point_i get_position(int offet) const
    {
        return phi.get_position( offet );
    }

    //! Getter function for the discrete level-set function #phi.
    Matrix<PhiValue>& get_phi() { return phi; }
    const Matrix<PhiValue>& get_phi() const { return phi; }
    //! Getter function for the exterior boundary #l_out.
    ContourList& get_l_out() { return l_out; }
    const ContourList& get_l_out() const { return l_out; }
    //! Getter function for the interior boundary #l_in.
    ContourList& get_l_in() { return l_in; }
    const ContourList& get_l_in() const { return l_in; }

    size_t get_preallocation_size() const { return preallocation_size; }

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
    Matrix<PhiValue> phi;

    //! List of offset points representing the exterior boundary.
    ContourList l_out;
    //! List of offset points representing the interior boundary.
    ContourList l_in;

    //! Preallocation size for each list.
    size_t preallocation_size;
};

/// Discrete sign of level-set function.
/// Returns -1 for interior side, +1 for exterior side.
/// Note: zero level-set is conceptual and never stored.
inline int phiSign(PhiValue v)
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

inline bool isInside(PhiValue v)
{
    return phiSign(v) < 0;
}

inline bool isOutside(PhiValue v)
{
    return !isInside(v);
}

inline bool differentSide(PhiValue a, PhiValue b)
{
    return phiSign(a) * phiSign(b) < 0;
}

inline bool ContourData::is_redundant(const ContourPoint& point) const
{
    int offset = point.get_offset();
    int x = point.get_x();
    int w = phi.get_width();
    int h = phi.get_height();
    int last_row_offset = w * (h - 1);

    const auto phi_center = phi[offset];

    // Voisins horizontaux
    if (x > 0)
    {
        int left_offset = offset - 1;

        if ( differentSide(phi[left_offset], phi_center) )
            return false;
    }

    if (x < w - 1)
    {
        int right_offset = offset + 1;
        if ( differentSide(phi[right_offset], phi_center) )
            return false;
    }

    // Voisins verticaux
    if (offset >= w) // Pas dans la première ligne
    {
        int up_offset = offset - w;
        if ( differentSide( phi[up_offset], phi_center ) )
            return false;
    }

    if (offset < last_row_offset) // Pas dans la dernière ligne
    {
        int down_offset = offset + w;
        if ( differentSide( phi[down_offset], phi_center ) )
            return false;
    }

#ifdef ALGO_8_CONNEXITY
    // Diagonaux supérieurs
    if (x > 0 && offset >= w)
    {
        int up_left_offset = offset - w - 1;
        if ( differentSide( phi[up_left_offset], phi_center ) )
            return false;
    }

    if (x < w - 1 && offset >= w)
    {
        int up_right_offset = offset - w + 1;
        if ( differentSide( phi[up_right_offset], phi_center ) )
            return false;
    }

    // Diagonaux inférieurs
    if (x > 0 && offset < last_row_offset)
    {
        int down_left_offset = offset + w - 1;
        if ( differentSide( phi[down_left_offset], phi_center ) )
            return false;
    }

    if (x < w - 1 && offset < last_row_offset)
    {
        int down_right_offset = offset + w + 1;
        if ( differentSide(phi[down_right_offset], phi_center) )
            return false;
    }
#endif

    return true;
}

}

#endif // CONTOUR_DATA_HPP
