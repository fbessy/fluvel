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
#include <iostream>

#include "contour_data.hpp"
#include "boundary_builder.hpp"

namespace ofeli_ip
{

ContourData::ContourData(int phi_width, int phi_height)
    : phi_(phi_width, phi_height)
{
    assert( phi_width  >= 1 );
    assert( phi_height >= 1 );

    allocate_lists();
    initialize_with_one_ellipse();
}

ContourData::ContourData(const unsigned char* phi_grayscale_img_data,
                         int phi_width, int phi_height)
    : phi_(phi_width, phi_height)
{
    assert( phi_grayscale_img_data != nullptr );
    assert( phi_width  >= 1 );
    assert( phi_height >= 1 );


    for( int offset = 0; offset < phi_.size(); ++offset )
    {
        if ( phi_grayscale_img_data[offset] >= 128u )
        {
            phi_[offset] = PhiValue::InsideRegion;
        }
        else
        {
            phi_[offset] = PhiValue::OutsideRegion;
        }
    }

    const int n = static_cast<int>(phi_.size());

    for ( int offset = 0; offset < n; ++offset )
    {
        const Point2D_i p = phi_.coord(offset);

        if ( is_redundant( { offset, p.x } ) )
            continue;

        PhiValue& v = phi_[offset];

        if ( v == PhiValue::InsideRegion )
        {
            v = PhiValue::InteriorBoundary;
            l_in_.emplace_back(offset, p.x);
        }
        else
        {
            v = PhiValue::ExteriorBoundary;
            l_out_.emplace_back(offset, p.x);
        }
    }

    repair_lists_if_needed();
}

ContourData::ContourData(const ContourList& l_out,
                         const ContourList& l_in,
                         int phi_width, int phi_height)
    : phi_(phi_width, phi_height), l_out_(l_out), l_in_(l_in)
{
    assert( !l_out.empty() );
    assert( !l_in.empty() );
    assert( phi_width  >= 1 );
    assert( phi_height >= 1 );

    allocate_lists();
    repair_lists_if_needed();
    define_phi_from_lists();
}

ContourData::ContourData(const ContourData& contour)
    : phi_( contour.phi_ ),
    l_out_( contour.l_out_ ),
    l_in_( contour.l_in_ )
{
    allocate_lists();
}

ContourData::ContourData(ContourData&& contour) noexcept
    : phi_( std::move(contour.phi_) ),
    l_out_( std::move(contour.l_out_) ),
    l_in_( std::move(contour.l_in_) )
{
    allocate_lists();
}

void ContourData::allocate_lists()
{
    const size_t perimeter        = 2*( phi_.width() + phi_.height() );
    const size_t elem_alloc_size_ = 3 * perimeter;

    l_out_.reserve( elem_alloc_size_ );
    l_in_.reserve( elem_alloc_size_ );
}

void ContourData::repair_lists_if_needed()
{
    if ( !l_out_.empty() && !l_in_.empty() )
        return;

    std::cerr << "\n ==> " << __FILE__ << " | " << __FUNCTION__ << " | " << __LINE__
              << "\nInvalid contour lists, rebuilding with ellipse.";

    initialize_with_one_ellipse();
}

void ContourData::initialize_with_one_ellipse()
{
    l_out_.clear();
    l_in_.clear();

    BoundaryBuilder init( phi_.width(), phi_.height(),
                          l_out_, l_in_ );

    init.get_ellipse_points( 0.5f, 0.5f, 0.4f, 0.4f );
    define_phi_from_lists();

    assert( !l_out_.empty() );
    assert( !l_in_.empty() );
}

void ContourData::define_phi_from_lists()
{
    phi_.fill(PhiValue::OutsideRegion);

    for( const auto& p : l_out_ )
    {
        phi_[ p.offset() ] = PhiValue::ExteriorBoundary;
    }

    for( const auto& p : l_in_ )
    {
        flood_fill( p.offset(),
                       PhiValue::OutsideRegion,
                       PhiValue::InsideRegion );

        phi_[ p.offset() ] = PhiValue::InteriorBoundary;
    }
}

void ContourData::flood_fill(int offset_seed,
                                PhiValue target_value,
                                PhiValue replacement_value)
{
    if( target_value != replacement_value &&
        offset_seed < phi_.width()*phi_.height()-1 )
    {
        std::stack<size_t> offset_seeds;
        // top seed coordinates (x_ts,y_ts) and x for scan the row
        int x;
        bool span_up, span_down;

        offset_seeds.push(offset_seed);

        while( !offset_seeds.empty() )
        {
            // unstack the top seed
            const auto [x_ts, y_ts] = phi_.coord( offset_seeds.top() );

            offset_seeds.pop();

            // x initialization at the left-most point of the seed
            x = x_ts;
            while( x > 0 && phi_.at(x-1,y_ts) == target_value )
            {
                x--;
            }

            span_up = false;
            span_down = false;

            // pixels are treated row-wise
            while( x < phi_.width() &&
                   phi_.at(x,y_ts) == target_value )
            {
                phi_.at(x,y_ts) = replacement_value;

                if( !span_up &&
                    y_ts > 0
                    && phi_.at(x,y_ts-1) == target_value )
                {
                    offset_seeds.push( phi_.offset(x,y_ts-1) );
                    span_up = true;
                }
                else if( span_up &&
                         y_ts > 0 &&
                         phi_.at(x,y_ts-1) != target_value )
                {
                    span_up = false;
                }

                if( !span_down &&
                    y_ts < phi_.height()-1 &&
                    phi_.at(x,y_ts+1) == target_value )
                {
                    offset_seeds.push( phi_.offset(x,y_ts+1) );
                    span_down = true;
                }
                else if( span_down &&
                         y_ts < phi_.height()-1 &&
                         phi_.at(x,y_ts+1) != target_value )
                {
                    span_down = false;
                }

                x++;
            }
        }
    }
}

bool ContourData::is_redundant(const ContourPoint& point) const
{
    const int offset = point.offset();
    const int x = point.x();
    const int w = phi_.width();
    const int h = phi_.height();
    const int last_row_offset = w * (h - 1);

    const auto phi_center = phi_[offset];

    // Voisins horizontaux
    if (x > 0)
    {
        int left_offset = offset - 1;

        if ( phi_value::differentSide(phi_[left_offset], phi_center) )
            return false;
    }

    if (x < w - 1)
    {
        int right_offset = offset + 1;
        if ( phi_value::differentSide(phi_[right_offset], phi_center) )
            return false;
    }

    // Voisins verticaux
    if (offset >= w) // Pas dans la première ligne
    {
        int up_offset = offset - w;
        if ( phi_value::differentSide( phi_[up_offset], phi_center ) )
            return false;
    }

    if (offset < last_row_offset) // Pas dans la dernière ligne
    {
        int down_offset = offset + w;
        if ( phi_value::differentSide( phi_[down_offset], phi_center ) )
            return false;
    }

#ifdef ALGO_8_CONNEXITY
    // Diagonaux supérieurs
    if (x > 0 && offset >= w)
    {
        int up_left_offset = offset - w - 1;
        if ( differentSide( phi_[up_left_offset], phi_center ) )
            return false;
    }

    if (x < w - 1 && offset >= w)
    {
        int up_right_offset = offset - w + 1;
        if ( differentSide( phi_[up_right_offset], phi_center ) )
            return false;
    }

    // Diagonaux inférieurs
    if (x > 0 && offset < last_row_offset)
    {
        int down_left_offset = offset + w - 1;
        if ( differentSide( phi_[down_left_offset], phi_center ) )
            return false;
    }

    if (x < w - 1 && offset < last_row_offset)
    {
        int down_right_offset = offset + w + 1;
        if ( differentSide(phi_[down_right_offset], phi_center) )
            return false;
    }
#endif

    return true;
}

}
