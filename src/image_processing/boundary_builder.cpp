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

#include <iostream>
#include "boundary_builder.hpp"

namespace ofeli_ip
{

BoundaryBuilder::BoundaryBuilder(int phi_width1, int phi_height1,
                                 ContourList& l_out_init1,
                                 ContourList& l_in_init1)
    : phi_width(phi_width1), phi_height(phi_height1),
    Lout_init(l_out_init1), Lin_init(l_in_init1)
{
    if( phi_width <= 0 || phi_height <= 0 )
    {
        std::cerr << std::endl <<
        " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
        "phi_width or phi_height should be strictly positive";
    }
}

void BoundaryBuilder::clear_lists()
{
    Lout_init.clear();
    Lin_init.clear();
}

void BoundaryBuilder::get_rectangle_points(int x1, int y1,
                                           int x2, int y2)
{
    get_rectangle_points( x1, y1,
                          x2, y2,
                          Lout_init, Lin_init );
}

void BoundaryBuilder::get_rectangle_points(float x1, float y1,
                                           float x2, float y2)
{
    get_rectangle_points( int(x1*phi_width), int(y1*phi_height),
                          int(x2*phi_width), int(y2*phi_height),
                          Lout_init, Lin_init );
}

void BoundaryBuilder::get_reversed_rectangle_points(int x1, int y1,
                                                    int x2, int y2)
{
    get_rectangle_points( x1, y1,
                          x2, y2,
                          Lin_init, Lout_init);
}

void BoundaryBuilder::get_reversed_rectangle_points(float x1, float y1,
                                                    float x2, float y2)
{
    get_rectangle_points( int(x1*phi_width), int(y1*phi_height),
                          int(x2*phi_width), int(y2*phi_height),
                          Lout_init, Lin_init);
}

void BoundaryBuilder::get_rectangle_points(int x1, int y1,
                                           int x2, int y2,
                                           ContourList& list_out,
                                           ContourList& list_in)
{
    if( phi_width != 0 && phi_height != 0 )
    {
        if( x1 > x2 )
        {
            std::swap(x1,x2);
        }
        if( y1 > y2 )
        {
            std::swap(y1,y2);
        }

        if( x1 != x2 && y1 != y2 )
        {
            get_rectangle_points_for_one_list( list_in, x1, y1, x2, y2 );

#ifdef ALGO_8_CONNEXITY
            get_rectangle_points_for_one_list( list_out, x1-1, y1-1, x2+1, y2+1 );
#else
            for( int x = x1; x <= x2; x++ )
            {
                if( x >= 0 && x < int(phi_width) )
                {
                    if( y1-1 >= 0 && y1-1 < int(phi_height) )
                    {
                        list_out.emplace_back( get_offset(x,y1-1), x );
                    }
                    if( y2+1 >= 0 && y2+1 < int(phi_height) )
                    {
                        list_out.emplace_back( get_offset(x,y2+1), x );
                    }
                }
            }

            for( int y = y1; y <= y2; y++ )
            {
                if( y >= 0 && y < int(phi_height) )
                {
                    if( x1-1 >= 0 && x1-1 < int(phi_width) )
                    {
                        list_out.emplace_back( get_offset(x1-1,y), x1-1 );
                    }

                    if( x2+1 >= 0 && x2+1 < int(phi_width) )
                    {
                        list_out.emplace_back( get_offset(x2+1,y), x2+1 );
                    }
                }
            }
#endif

        }
        else
        {
            std::cerr << std::endl <<
            " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
            "Point 1 and 2 should not be the same.";
        }
    }
}

void BoundaryBuilder::get_rectangle_points_for_one_list(ContourList& list_init,
                                                        int x1, int y1,
                                                        int x2, int y2)
{
    for( int x = x1; x <= x2; x++ )
    {
        if( x >= 0 && x < int(phi_width) )
        {
            if( y1 >= 0 && y1 < int(phi_height) )
            {
                list_init.emplace_back( get_offset(x,y1), x );
            }
            if( y2 >= 0 && y2 < int(phi_height) )
            {
                list_init.emplace_back( get_offset(x,y2), x );
            }
        }
    }

    for( int y = y1+1; y < y2; y++ )
    {
        if( y >= 0 && y < int(phi_height) )
        {
            if( x1 >= 0 && x1 < int(phi_width) )
            {
                list_init.emplace_back( get_offset(x1,y), x1 );
            }

            if( x2 >= 0 && x2 < int(phi_width) )
            {   
                list_init.emplace_back( get_offset(x2,y), x2 );
            }
        }
    }
}

void BoundaryBuilder::build_ellipse_midpoint_connected(
    int x0, int y0,
    unsigned int a, unsigned int b,
    ContourList& list_out)
{
    int x = 0;
    int y = b;

    // Carrés
    long a2 = (long)a * a;
    long b2 = (long)b * b;

    long dx = 0;
    long dy = 2 * a2 * y;

    // Paramètre de décision région 1
    long d1 = b2 - a2 * b + a2 / 4;

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

        if (abs(dxp) == 1 && abs(dyp) == 1) {
            // pixel correcteur (choix simple et sûr)
            add_4_points_in_ellipse(list_out, prev_x, y, x0, y0);
        }

        prev_x = x;
        prev_y = y;

        x++;
        dx += 2 * b2;

        if (d1 < 0) {
            d1 += dx + b2;
        } else {
            y--;
            dy -= 2 * a2;
            d1 += dx - dy + b2;
        }
    }

    // Paramètre région 2
    long d2 = b2 * (x + 0.5) * (x + 0.5)
              + a2 * (y - 1) * (y - 1)
              - a2 * b2;

    // -------- Région 2 --------
    while (y >= 0)
    {
        add_4_points_in_ellipse(list_out, x, y, x0, y0);

        // ---- Connexité 8 ----
        int dxp = x - prev_x;
        int dyp = y - prev_y;

        if (abs(dxp) == 1 && abs(dyp) == 1) {
            add_4_points_in_ellipse(list_out, x, prev_y, x0, y0);
        }

        prev_x = x;
        prev_y = y;

        y--;
        dy -= 2 * a2;

        if (d2 > 0) {
            d2 += a2 - dy;
        } else {
            x++;
            dx += 2 * b2;
            d2 += dx - dy + a2;
        }
    }
}

void BoundaryBuilder::build_inner_contiguous(
    int x0, int y0,
    const ContourList& l_out,
    ContourList& l_in)
{
    int x, y;
    int dx, dy;
    int sx, sy;
    int xi, yi;

    for( std::size_t i = 0; i < l_out.size(); i++ )
    {
        get_position( l_out[i].get_offset(), x, y);

        dx = x0 - x;
        dy = y0 - y;

        sx = (dx > 0) - (dx < 0);
        sy = (dy > 0) - (dy < 0);

        xi = x + sx;
        yi = y + sy;

        if ( inside_image(xi, yi) )
        {
            l_in.emplace_back( get_offset(xi, yi), xi );
        }
    }
}

void BoundaryBuilder::get_ellipse_points(int x0, int y0,
                                         unsigned int a, unsigned int b,
                                         ContourList& list_out,
                                         ContourList& list_in)
{

    build_ellipse_midpoint_connected(x0, y0, a, b, list_out);
    build_inner_contiguous(x0, y0, list_out, list_in);



#if 0
    List_i list_in_temp, list_out_temp;

    const long long int aa = (long long int)(a*a);
    const long long int bb = (long long int)(b*b);
    const long long int aa2 = 2*aa;
    const long long int bb2 = 2*bb;

    long long int x = 0;
    long long int y = (long long int)(b);

    add_4_points_in_ellipse( list_in_temp,
                             int(x), int(y),
                             x0, y0 );

    long long int dpx = 0;
    long long int dpy = aa2*y;

    long long int dp = (long long int)( 0.5f+bb-(aa*(long long int)(b))+(aa/4) );

    while( dpx < dpy )
    {
        x++;
        dpx += bb2;
        if( dp < 0 )
        {
            dp += bb+dpx;
        }
        else
        {
            y--;
            dpy -= aa2;
            dp += bb+dpx-dpy;
        }

        add_4_points_in_ellipse( list_in_temp,
                                 int(x), int(y),
                                 x0, y0 );
    }

    dp = (long long int)( 0.5+bb*(x+0.5)*(x+0.5)+aa*(y-1)*(y-1)-aa*bb );

    while( y > 0 )
    {
        y--;
        dpy -= aa2;

        if( dp > 0 )
        {
            dp += aa-dpy;
        }
        else
        {
            x++;
            dpx += bb2;
            dp += aa-dpy+dpx;
        }

        add_4_points_in_ellipse( list_in_temp,
                                 int(x), int(y),
                                 x0, y0 );
    }

    int pos_x, pos_y;

    for( auto it = list_in_temp.cbegin(); it != list_in_temp.cend(); ++it )
    {
        get_position(*it, pos_x, pos_y);

        if( pos_x-1 >= 0 )
        {
            if(   square( (float(pos_x-1)-float(x0))/2.f ) / square( float(a)/2.f )
                  + square( (float(pos_y)-float(y0))/2.f ) / square( float(b)/2.f )
                  > 1.f
                  )
            {
                list_out_temp.emplace_back( get_offset(pos_x-1,pos_y) );
                list_out_temp.sort();
            }
        }

        if( pos_x+1 < phi_width )
        {
            if(   square( (float(pos_x+1)-float(x0))/2.f ) / square( float(a)/2.f )
                  + square( (float(pos_y)-float(y0))/2.f ) / square( float(b)/2.f )
                  > 1.f
                  )
            {
                list_out_temp.emplace_back( get_offset(pos_x+1,pos_y) );
                list_out_temp.sort();
            }
        }

        if( pos_y-1 >= 0 )
        {
            if(   square( (float(pos_x)-float(x0))/2.f ) / square( float(a)/2.f )
                  + square( (float(pos_y-1)-float(y0))/2.f ) / square( float(b)/2.f )
                  > 1.f
                  )
            {
                list_out_temp.emplace_back( get_offset(pos_x,pos_y-1) );
                list_out_temp.sort();
            }

#ifdef ALGO_8_CONNEXITY
            if( pos_x-1 >= 0 )
            {
                if(   square( (float(pos_x-1)-float(x0))/2.f ) / square( float(a)/2.f )
                        + square( (float(pos_y-1)-float(y0))/2.f ) / square( float(b)/2.f )
                    > 1.f
                    )

                {
                    list_out_temp.emplace_back( get_offset(x_ui-1,y_ui-1) );
                    list_out_temp.sort();
                }
            }

            if( pos_x+1 < phi_width )
            {
                if(   square( (float(pos_x+1)-float(x0))/2.f ) / square( float(a)/2.f )
                        + square( (float(pos_y-1)-float(y0))/2.f ) / square( float(b)/2.f )
                    > 1.f
                    )

                {
                    list_out_temp.emplace_back( get_offset(x_ui+1,y_ui-1) );
                    list_out_temp.sort();
                }
            }
#endif
        }

        if( pos_y+1 < phi_height )
        {
            if(   square( (float(pos_x)-float(x0))/2.f ) / square( float(a)/2.f )
                  + square( (float(pos_y+1)-float(y0))/2.f ) / square( float(b)/2.f )
                  > 1.f
                  )

            {
                list_out_temp.emplace_back( get_offset(pos_x,pos_y+1) );
                list_out_temp.sort();
            }

#ifdef ALGO_8_CONNEXITY
            if( pos_x-1 >= 0 )
            {
                if(   square( (float(pos_x-1)-float(x0))/2.f ) / square( float(a)/2.f )
                        + square( (float(pos_y+1)-float(y0))/2.f ) / square( float(b)/2.f )
                    > 1.f
                    )

                {
                    list_out_temp.emplace_back( get_offset(pos_x-1,pos_y+1) );
                    list_out_temp.sort();
                }
            }

            if( pos_x+1 < phi_width )
            {

                if(   square( (double(pos_x+1)-double(x0))/2.f ) / square( double(a)/2.f )
                        + square( (double(pos_y+1)-double(y0))/2.f ) / square( double(b)/2.f )
                    > 1.f
                    )

                {
                    list_out_temp.emplace_back( get_offset(pos_x+1,pos_y+1) );
                    list_out_temp.sort();
                }
            }
#endif

        }
    }

    list_out_temp.unique();

    for( auto it = list_in_temp.cbegin(); it != list_in_temp.cend(); ++it )
    {
        list_out_temp.remove(*it);
    }

    for( auto it = list_out_temp.cbegin(); it != list_out_temp.cend(); ++it )
    {
        list_out.emplace_back(*it);
    }
    for( auto it = list_in_temp.cbegin(); it != list_in_temp.cend(); ++it )
    {
        list_in_temp.emplace_back(*it);
    }

    //list_out.emplace_back(list_out_temp);
    //list_in.emplace_back(list_in_temp);

    list_out.splice( list_out.end(), list_out_temp );
    list_in.splice( list_in.end(), list_in_temp );
#endif

}

void BoundaryBuilder::get_ellipse_points(int x0, int y0,
                                         unsigned int a, unsigned int b)
{
    get_ellipse_points( x0, y0,
                        a, b,
                        Lout_init, Lin_init );
}

void BoundaryBuilder::get_reversed_ellipse_points(int x0, int y0,
                                                  unsigned int a, unsigned int b)
{
    get_ellipse_points( x0, y0,
                        a, b,
                        Lin_init, Lout_init );
}

void BoundaryBuilder::get_ellipse_points(float x0, float y0,
                                         float a,  float b)
{
    get_ellipse_points( int(x0*phi_width), int(y0*phi_height),
                        (unsigned int)(a*phi_width), (unsigned int)(b*phi_height),
                        Lout_init, Lin_init );
}

void BoundaryBuilder::get_reversed_ellipse_points(float x0, float y0,
                                                  float a, float b)
{
    get_ellipse_points( int(x0*phi_width), int(y0*phi_height),
                        (unsigned int)(a*phi_width), (unsigned int)(b*phi_height),
                        Lin_init, Lout_init );
}

}
