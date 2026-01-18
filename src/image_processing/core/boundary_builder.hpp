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

#ifndef BOUNDARY_BUILDER_H
#define BOUNDARY_BUILDER_H

#include <vector>
#include "contour_data.hpp"

// false or true below, respectively,
// if you want to use a matrix with a row/column wise data buffer
#ifndef IS_COLUMN_WISE
#define IS_COLUMN_WISE false
#endif
// functions affected by the define :
// - unsigned int get_offset(unsigned int x, unsigned int y) const
// - void get_position(unsigned int offset, unsigned int& x, unsigned int& y) const

namespace ofeli_ip
{

class BoundaryBuilder
{
public :

    BoundaryBuilder(int phi_width1, int phi_height1,
                    ContourList& l_out_init1,
                    ContourList& l_in_init1);

    //! Clears the both lists.
    void clear_lists();

    //! Gets rectangle points with boundary box (x1,y1) and (x2,y2).
    void get_rectangle_points(int x1, int y1,
                              int x2, int y2);

    //! Gets rectangle points with boundary box (x1,y1) and (x2,y2).
    void get_reversed_rectangle_points(int x1, int y1,
                                       int x2, int y2);

    //! Gets rectangle points with normalized boundary box (x1,y1) and (x2,y2).
    void get_rectangle_points(float x1, float y1,
                              float x2, float y2);

    //! Gets rectangle points with normalized boundary box (x1,y1) and (x2,y2).
    void get_reversed_rectangle_points(float x1, float y1,
                                       float x2, float y2);

    //! Gets ellipse points with center point and width and height.
    void get_ellipse_points(int x0, int y0,
                            unsigned int a, unsigned int b);

    //! Gets ellipse points with center point and width and height.
    void get_reversed_ellipse_points(int x0, int y0,
                                     unsigned int a, unsigned int b);

    //! Gets ellipse points with normalized center point and width and height.
    void get_ellipse_points(float x0, float y0,
                            float a, float b);

    //! Gets ellipse points with normalized center point and width and height.
    void get_reversed_ellipse_points(float x0, float y0,
                                     float a, float b);

private :

    void get_rectangle_points(int x1, int y1,
                              int x2, int y2,
                              ContourList& list_out,
                              ContourList& list_in);

    void get_rectangle_points_for_one_list(ContourList& list_init,
                                           int x1, int y1,
                                           int x2, int y2);

    void build_ellipse_midpoint_connected(int x0, int y0,
                                          unsigned int a, unsigned int b,
                                          ContourList& list_out);

    void build_inner_contiguous(int x0, int y0,
                                const ContourList& l_out,
                                ContourList& l_in);

    //! Gets ellipse points with center point and width and height.
    void get_ellipse_points(int x0, int y0,
                            unsigned int a, unsigned int b,
                            ContourList& list_out,
                            ContourList& list_in);

    void add_4_points_in_ellipse(ContourList& list_init,
                                 int x, int y,
                                 int x0, int y0);

    int get_offset(int x, int y) const;

    void get_position(int offset,
                      int& x, int& y) const;

    inline bool inside_image(int x, int y) {
        return x >= 0 && x < phi_width &&
               y >= 0 && y < phi_height;
    }


    int phi_width;
    int phi_height;

    ContourList& Lout_init;
    ContourList& Lin_init;
};

inline int BoundaryBuilder::get_offset(int x, int y) const
{
    if( IS_COLUMN_WISE )
    {
        return x*phi_height+y;
    }
    else
    {
        return x+y*phi_width;
    }
}

inline void BoundaryBuilder::get_position(int offset,
                                          int& x, int& y) const
{
    if( IS_COLUMN_WISE )
    {
        x = offset/phi_height;
        y = offset-x*phi_height;
    }
    else
    {
        y = offset/phi_width;
        x = offset-y*phi_width;
    }
}

inline void BoundaryBuilder::add_4_points_in_ellipse(ContourList& list,
                                                     int x, int y,
                                                     int x0, int y0)
{
    if( x0-x >= 0 &&
        x0-x < phi_width &&
        y0-y >= 0 &&
        y0-y < phi_height )
    {
        list.emplace_back( get_offset(x0-x,y0-y), x0-x );
    }

    if( x0-x >= 0 &&
        x0-x < phi_width &&
        y0+y >= 0 &&
        y0+y < phi_height )
    {
        list.emplace_back( get_offset(x0-x,y0+y), x0-x );
    }

    if( x0+x >= 0 &&
        x0+x < phi_width &&
        y0-y >= 0 &&
        y0-y < phi_height )
    {
        list.emplace_back( get_offset(x0+x,y0-y), x0+x );
    }

    if( x0+x >= 0 &&
        x0+x < phi_width &&
        y0+y >= 0 &&
        y0+y < phi_height )
    {
        list.emplace_back( get_offset(x0+x,y0+y), x0+x );
    }
}

}

#endif // BOUNDARY_BUILDER_HPP
