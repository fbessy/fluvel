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

#ifndef CONTOUR_DATA_H
#define CONTOUR_DATA_H

#include <vector>

#include "matrix.hpp"
#include "list_i.hpp"

namespace ofeli_ip
{

enum PhiValue : signed char
{
    INSIDE_REGION = -3,
    INTERIOR_BOUNDARY = -1,
    ZERO_LEVEL_SET = 0,
    EXTERIOR_BOUNDARY = 1,
    OUTSIDE_REGION = 3
};

enum SpeedValue : signed char
{
    GO_INWARD  = -1,
    NO_MOVE    =  0,
    GO_OUTWARD =  1
};

struct ContourPoint
{
    int offset;
    signed char speed;
};

class ContourList
{

public:
    ContourList(int phi_width1, int phi_height1):
        phi_width(phi_width1),
        phi_height(phi_height1)
    {
    }

    void get_point(int index,
                   int& x, int& y) const
    {
        int offset = points[index].offset;
        y = offset/phi_width;
        x = offset-y*phi_width;
    }

    int get_point(int index) const
    {
        return points[index].offset;
    }

    int size() const
    {
        return points.size();
    }

private:
    std::vector<ContourPoint> points;
    int phi_width;
    int phi_height;
};

class ContourData
{

public :

    //! Constructor to initialize the contour with one ellipse.
    ContourData(int phi_width, int phi_height);

    //! Constructor to initialize the contour from a grayscale image data buffer of the levet-set function #phi.
    ContourData(const unsigned char* phi_grayscale_img_data,
                int phi_width, int phi_height);

    //! Constructor to initialize the contour with the both neighbouring boundaries lists of #l_out and #l_in.
    ContourData(const List_i& l_out1,
                const List_i& l_in1,
                int phi_width, int phi_height);

    //! Copy constructor.
    ContourData(const ContourData& contour);

    //! Move constructor.
    ContourData(ContourData&& contour) noexcept;

    //! Checks if a given point is redundant to define a boundary, i.e. if no neighbors have a different phi value sign comparing to the given point.
    bool is_boundary_redundant(int offset) const;

    //! Checks lists.
    bool check_lists();

    //! Wrapper to use directly with offset lists without the need to get the variable #phi.
    Point_i get_position(int offet) const
    {
        return phi.get_position( offet );
    }

    //! Getter function for the discrete level-set function #phi.
    Matrix<PhiValue>& get_phi() { return phi; }
    const Matrix<PhiValue>& get_phi() const { return phi; }
    //! Getter function for the exterior boundary #l_out.
    List_i& get_l_out() { return l_out; }
    const List_i& get_l_out() const { return l_out; }
    //! Getter function for the interior boundary #l_in.
    List_i& get_l_in() { return l_in; }
    const List_i& get_l_in() const { return l_in; }

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
    List_i l_out;
    //! List of offset points representing the interior boundary.
    List_i l_in;

    //std::vector<int> l_out2;
    //std::vector<int> l_in2;

    //std::vector<int> points_to_append;
};

inline bool ContourData::is_boundary_redundant(int offset) const
{
    int x, y;
    phi.get_position(offset,x,y); // x and y passed by reference

    if( x-1 >= 0 )
    {
        if( phi(x-1,y)*phi[offset] < 0 )
        {
            return false;
        }
    }
    if( x+1 < phi.get_width() )
    {
        if( phi(x+1,y)*phi[offset] < 0 )
        {
            return false;
        }
    }

    if( y-1 >= 0 )
    {
        if( phi(x,y-1)*phi[offset] < 0 )
        {
            return false;
        }

#ifdef ALGO_8_CONNEXITY
        if( x-1 >= 0 )
        {
            if( phi(x-1,y-1)*phi[offset] < 0 )
            {
                return false;
            }
        }
        if( x+1 < phi.get_width() )
        {
            if( phi(x+1,y-1)*phi[offset] < 0 )
            {
                return false;
            }
        }
#endif

    }

    if( y+1 < phi.get_height() )
    {
        if( phi(x,y+1)*phi[offset] < 0 )
        {
            return false;
        }

#ifdef ALGO_8_CONNEXITY
        if( x-1 >= 0 )
        {
            if( phi(x-1,y+1)*phi[offset] < 0 )
            {
                return false;
            }
        }
        if( x+1 < phi.get_width() )
        {
            if( phi(x+1,y+1)*phi[offset] < 0 )
            {
                return false;
            }
        }
#endif

    }

    return true;
}

}

#endif // CONTOUR_DATA_H
