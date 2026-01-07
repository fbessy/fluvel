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

#include <stack>

#include "contour_data.hpp"
#include "boundary_builder.hpp"

namespace ofeli_ip
{

bool ContourData::is_ok_phi_dimension(int phi_dimension)
{
    bool is_ok = true;

    if( phi_dimension <= 0 )
    {
        std::cerr << std::endl <<
            " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
            "phi_dimension must be be strictly positive.";

        is_ok = false;
    }

    return is_ok;
}

bool ContourData::check_lists()
{
    bool is_ok = true;

    if( l_out.empty() )
    {
        std::cerr << std::endl <<
            " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
            "l_out is empty.";

        is_ok = false;
    }

    if( l_in.empty() )
    {
        std::cerr << std::endl <<
            " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
            "l_in is empty.";

        is_ok = false;
    }

    if ( !is_ok )
    {
        initialize_with_one_ellipse();
    }

    return is_ok;
}

void ContourData::allocate_lists()
{
    size_t perimeter = 2*( phi.get_width() + phi.get_height() );
    preallocation_size = 3 * perimeter;

    l_out.reserve( preallocation_size );
    l_in.reserve( preallocation_size );
}

ContourData::ContourData(int phi_width, int phi_height)
    : phi(phi_width, phi_height)
{
    allocate_lists();

    if (    is_ok_phi_dimension( phi_width )
         && is_ok_phi_dimension( phi_height ) )
    {
        initialize_with_one_ellipse();
    }
}

ContourData::ContourData(const unsigned char* phi_grayscale_img_data,
                         int phi_width, int phi_height)
    : phi(phi_width, phi_height)
{
    allocate_lists();

    bool is_ok_ptr = true;
    int x, y;

    if( phi_grayscale_img_data == nullptr )
    {
        std::cerr << std::endl <<
            " ==> " <<  __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__ << std::endl <<
            "phi_grayscale_img_data must be a non-null pointer, it must be allocated.";

        is_ok_ptr = false;
    }

    if (    is_ok_ptr
         && is_ok_phi_dimension( phi_width )
         && is_ok_phi_dimension( phi_height ) )
    {
        int phi_size = phi_width*phi_height;

        for( int offset = 0; offset < phi_size; offset++ )
        {
            if ( phi_grayscale_img_data[offset] >= 128u )
            {
                phi[offset] = PhiValue::INSIDE_REGION;
            }
            else
            {
                phi[offset] = PhiValue::OUTSIDE_REGION;
            }
        }

        for( int offset = 0; offset < phi_size; offset++ )
        {
            phi.get_position(offset, x, y);

            if ( !is_redundant( {offset, x} ) )
            {
                if ( phi[offset] == PhiValue::INSIDE_REGION )
                {
                    phi[offset] = PhiValue::INTERIOR_BOUNDARY;
                    l_in.emplace_back( offset, x );
                }
                else
                {
                    phi[offset] = EXTERIOR_BOUNDARY;
                    l_out.emplace_back( offset, x );
                }
            }
        }

        check_lists();
    }
}

void ContourData::initialize_with_one_ellipse()
{
    l_out.clear();
    l_in.clear();

    BoundaryBuilder init( phi.get_width(), phi.get_height(),
                          l_out, l_in );

    init.get_ellipse_points( 0.5f, 0.5f, 0.4f, 0.4f );
    define_phi_with_boundary();
}

ContourData::ContourData(const ContourList& l_out1,
                         const ContourList& l_in1,
                         int phi_width, int phi_height)
    : phi(phi_width, phi_height), l_out(l_out1), l_in(l_in1)
{
    allocate_lists();

    if (    is_ok_phi_dimension( phi_width )
         && is_ok_phi_dimension( phi_height ) )
    {
        if ( check_lists() )
        {
            define_phi_with_boundary();
        }
    }
}

ContourData::ContourData(const ContourData& contour)
    : phi( contour.phi ),
    l_out( contour.l_out ),
    l_in( contour.l_in )
{
    allocate_lists();
}

ContourData::ContourData(ContourData&& contour) noexcept
    : phi( std::move(contour.phi) ),
    l_out( std::move(contour.l_out) ),
    l_in( std::move(contour.l_in) )
{
    allocate_lists();
}

void ContourData::define_phi_with_boundary()
{
    phi.memset(PhiValue::OUTSIDE_REGION);

    for( std::size_t i = 0; i < l_out.size(); i++ )
    {
        phi[ l_out[i].get_offset() ] = PhiValue::EXTERIOR_BOUNDARY;
    }

    for( std::size_t i = 0; i < l_in.size(); i++ )
    {
        do_flood_fill(l_in[i].get_offset(),
                      PhiValue::OUTSIDE_REGION,
                      PhiValue::INSIDE_REGION);

        phi[ l_in[i].get_offset() ] = PhiValue::INTERIOR_BOUNDARY;
    }
}

void ContourData::do_flood_fill(int offset_seed,
                                PhiValue target_value,
                                PhiValue replacement_value)
{
    if( target_value != replacement_value &&
        offset_seed < phi.get_width()*phi.get_height()-1 )
    {
        std::stack<int> offset_seeds;
        // top seed coordinates (x_ts,y_ts) and x for scan the row
        int x_ts, y_ts, x;
        bool span_up, span_down;

        offset_seeds.push(offset_seed);

        while( !offset_seeds.empty() )
        {
            // unstack the top seed
            phi.get_position(offset_seeds.top(),
                             x_ts,y_ts); // x_ts and y_ts passed by reference
            offset_seeds.pop();

            // x initialization at the left-most point of the seed
            x = x_ts;
            while( x > 0 && phi(x-1,y_ts) == target_value )
            {
                x--;
            }

            span_up = false;
            span_down = false;

            // pixels are treated row-wise
            while( x < phi.get_width() &&
                   phi(x,y_ts) == target_value )
            {
                phi(x,y_ts) = replacement_value;

                if( !span_up &&
                    y_ts > 0
                    && phi(x,y_ts-1) == target_value )
                {
                    offset_seeds.push( phi.get_offset(x,y_ts-1) );
                    span_up = true;
                }
                else if( span_up &&
                         y_ts > 0 &&
                         phi(x,y_ts-1) != target_value )
                {
                    span_up = false;
                }

                if( !span_down &&
                    y_ts < phi.get_height()-1 &&
                    phi(x,y_ts+1) == target_value )
                {
                    offset_seeds.push( phi.get_offset(x,y_ts+1) );
                    span_down = true;
                }
                else if( span_down &&
                         y_ts < phi.get_height()-1 &&
                         phi(x,y_ts+1) != target_value )
                {
                    span_down = false;
                }

                x++;
            }
        }
    }
}

}
