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
#include "neighborhood.hpp"

namespace ofeli_ip
{

ContourData::ContourData(int phi_width, int phi_height,
                         Connectivity connectivity)
    : phi_(phi_width, phi_height),
    connectivity_(connectivity)
{
    assert( phi_width  >= 1 );
    assert( phi_height >= 1 );

    allocate_lists();
    define_from_ellipse();


    // post condition
    assert( is_valid() );
}

ContourData::ContourData(const unsigned char* phi_grayscale_img_data,
                         int phi_width, int phi_height,
                         Connectivity connectivity)
    : phi_(phi_width, phi_height),
    connectivity_(connectivity)
{
    assert( phi_grayscale_img_data != nullptr );
    assert( phi_width  >= 1 );
    assert( phi_height >= 1 );


    for( size_t offset = 0; offset < phi_.size(); ++offset )
    {
        if ( phi_grayscale_img_data[ offset ] >= 128u )
        {
            phi_[offset] = PhiValue::InteriorBoundary;
        }
        else
        {
            phi_[offset] = PhiValue::ExteriorBoundary;
        }
    }

    define_lists_and_phi_from_binary_phi();

    if ( empty() )
        define_from_ellipse();
    else
        eliminate_redundant_points_if_needed();


    // post condition
    assert( is_valid() );
}

ContourData::ContourData(const RawContour& l_out,
                         const RawContour& l_in,
                         int phi_width, int phi_height,
                         Connectivity connectivity)
    : phi_(phi_width, phi_height), l_out_(l_out), l_in_(l_in),
    connectivity_(connectivity)
{
    assert( !l_out.empty() );
    assert( !l_in.empty() );
    assert( phi_width  >= 1 );
    assert( phi_height >= 1 );

    allocate_lists();

    if ( empty() )
        define_from_ellipse();
    else
        define_phi_from_lists();


    // post condition
    assert( is_valid() );
}

ContourData::ContourData(const ContourData& contour)
    : phi_( contour.phi_ ),
    l_out_( contour.l_out_ ),
    l_in_( contour.l_in_ ),
    connectivity_( contour.connectivity_ )
{
    allocate_lists();
}

ContourData::ContourData(ContourData&& contour) noexcept
    : phi_( std::move(contour.phi_) ),
    l_out_( std::move(contour.l_out_) ),
    l_in_( std::move(contour.l_in_) ),
    connectivity_( contour.connectivity_ )
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

void ContourData::define_from_ellipse()
{
    l_out_.clear();
    l_in_.clear();

    BoundaryBuilder lists_init( phi_.width(), phi_.height(),
                                l_out_, l_in_ );

    lists_init.generate_ellipse_points( 0.8f, 0.8f );

    define_phi_from_lists();
}

void ContourData::define_lists_and_phi_from_binary_phi()
{
    for ( size_t offset = 0; offset < phi_.size(); ++offset )
    {
        PhiValue& current_phi = phi_[offset];
        PhiValue region_val;
        RawContour* boundary = nullptr;
        bool is_boundary = false;

        // get the generic context to eliminate redundant points
        if ( current_phi == PhiValue::ExteriorBoundary )
        {
            region_val = PhiValue::OutsideRegion;
            boundary = &l_out_;
            is_boundary = true;
        }
        else if ( current_phi == PhiValue::InteriorBoundary )
        {
            region_val = PhiValue::InsideRegion;
            boundary = &l_in_;
            is_boundary = true;
        }

        if ( is_boundary )
        {
            const Point2D_i current_point = phi_.coord(offset);
            const ContourPoint point{ current_point.x, current_point.y };

            if ( is_redundant( point ) )
                current_phi = region_val;
            else
                boundary->push_back( point );
        }
    }
}

void ContourData::define_phi_from_lists()
{
    phi_.fill(PhiValue::OutsideRegion);

    for( const auto& p : l_out_ )
    {
        phi_.at( p.x, p.y ) = PhiValue::ExteriorBoundary;
    }

    for( const auto& p : l_in_ )
    {
        flood_fill( { p.x, p.y },
                    PhiValue::OutsideRegion,
                    PhiValue::InsideRegion );

        phi_.at( p.x, p.y ) = PhiValue::InteriorBoundary;
    }

    eliminate_redundant_points_if_needed();
}

void ContourData::flood_fill(const Point2D_i& seed,
                             PhiValue target_value,
                             PhiValue replacement_value)
{
    if( target_value != replacement_value &&
        phi_.valid(seed) )
    {
        std::stack<Point2D_i> seeds_stack;
        // top seed coordinates (x_ts,y_ts) and x for scan the row
        int x;
        bool span_up, span_down;

        seeds_stack.push(seed);

        while( !seeds_stack.empty() )
        {
            // unstack the top seed
            const auto [x_ts, y_ts] = seeds_stack.top();

            seeds_stack.pop();

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
                    seeds_stack.emplace( x, y_ts-1 );
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
                    seeds_stack.emplace( x, y_ts+1 );
                    span_down = true;
                }
                else if( span_down &&
                         y_ts < phi_.height()-1 &&
                         phi_.at(x,y_ts+1) != target_value )
                {
                    span_down = false;
                }

                ++x;
            }
        }
    }
}

void ContourData::eliminate_redundant_points_if_needed()
{
    eliminate_redundant_points(l_out_, PhiValue::OutsideRegion);
    eliminate_redundant_points(l_in_,  PhiValue::InsideRegion);
}

void ContourData::eliminate_redundant_points(RawContour& boundary,
                                             PhiValue region_value)
{
    for( std::size_t i = 0; i < boundary.size(); )
    {
        auto& point = boundary[i];

        if( is_redundant(point) )
        {
            phi_.at( point.x, point.y) = region_value;
            point = boundary.back();
            boundary.pop_back();
        }
        else
        {
            ++i;
        }
    }
}

bool ContourData::is_redundant(const ContourPoint& p) const
{
    const int x = p.x;
    const int y = p.y;

    const int w = phi_.width();
    const int h = phi_.height();

    const auto phi_center = phi_.at(x, y);
    const auto connectivity = connectivity_;

    if ( fully_inside_8(x, y, w, h) )
    {
        // FAST PATH: aucun valid(), aucune branche parasite

        for ( const auto& d : neighbors4 )
        {
            if ( phi_value::differentSide(phi_.at(x + d.dx, y + d.dy),
                                          phi_center) )
                return false;
        }

        if ( connectivity == Connectivity::Eight )
        {
            for ( const auto& d : neighbors4_diag )
            {
                if ( phi_value::differentSide(phi_.at(x + d.dx, y + d.dy),
                                              phi_center) )
                    return false;
            }
        }

        return true;
    }
    else
    {
        // SLOW PATH: bords

        for ( const auto& d : neighbors4 )
        {
            const int nx = x + d.dx;
            const int ny = y + d.dy;

            if ( !phi_.valid(nx, ny) )
                continue;

            if ( phi_value::differentSide(phi_.at(nx, ny),
                                          phi_center) )
                return false;
        }

        if ( connectivity == Connectivity::Eight )
        {
            for ( const auto& d : neighbors4_diag )
            {
                const int nx = x + d.dx;
                const int ny = y + d.dy;

                if ( !phi_.valid(nx, ny) )
                    continue;

                if ( phi_value::differentSide(phi_.at(nx, ny),
                                              phi_center) )
                    return false;
            }
        }

        return true;
    }
}

bool ContourData::is_valid() const
{
    if ( empty() )
        return false;

    for ( const auto& p : l_out_ )
    {
        if ( phi_.at( p.x, p.y ) != PhiValue::ExteriorBoundary )
            return false;

        if ( is_redundant( p ) )
            return false;
    }

    for ( const auto& p : l_in_ )
    {
        if ( phi_.at( p.x, p.y ) != PhiValue::InteriorBoundary )
            return false;

        if ( is_redundant( p ) )
            return false;
    }

    RawContour result;
    result.reserve( l_out_.size() + l_in_.size() );
    result.insert( result.end(), l_out_.begin(), l_out_.end() );
    result.insert( result.end(), l_in_.begin(), l_in_.end() );

    if ( has_duplicates(result) )
        return false;


    return true;
}

ExportedContour ContourData::export_contour(const RawContour& raw_boundary) const
{
    ExportedContour geometric_boundary;
    geometric_boundary.reserve( raw_boundary.size() );

    for ( const auto& point : raw_boundary )
    {
        geometric_boundary.emplace_back( point.x, point.y );
    }

    return geometric_boundary;
}

}
