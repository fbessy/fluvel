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

#include <cstdlib>     // std::abs
#include <cassert>

#include "active_contour.hpp"
#include "hausdorff_distance.hpp"


namespace ofeli_ip
{

// Definitions
ActiveContour::ActiveContour(const ContourData& initial_contour,
                             const AcConfig& config1)
    : config(config1),
    cd( initial_contour ),
    state( State::Cycle1 ),
    stopping_status( StoppingStatus::None ),
    ctx( cd ),
    ed( cd ),
    is_cycle2_condition( true )
{
    kernel = build_gaussian_kernel_linear(config.kernel_length,
                                          config.sigma,
                                          cd.get_phi().get_width());

    points_to_append.reserve( cd.get_preallocation_size() );

    if ( config.is_cycle2 )
    {
        n_iter_by_cycle = config.Na + config.Ns;
    }
    else
    {
        n_iter_by_cycle = config.Na;
    }
}

ActiveContour::ActiveContour(ContourData&& initial_contour,
                             const AcConfig& config1)
    : config(config1),
    cd( std::move(initial_contour) ),
    state( State::Cycle1 ),
    stopping_status( StoppingStatus::None ),
    ctx( cd ),
    ed( cd ),
    is_cycle2_condition(true)
{
    kernel = build_gaussian_kernel_linear(config.kernel_length,
                                          config.sigma,
                                          cd.get_phi().get_width());

    points_to_append.reserve( cd.get_preallocation_size() );

    if ( config.is_cycle2 )
    {
        n_iter_by_cycle = config.Na + config.Ns;
    }
    else
    {
        n_iter_by_cycle = config.Na;
    }
}

void ActiveContour::evolve()
{
    // Fast Two Cycle algorithm

    while( state != State::Stopped )
    {
        while( state == State::Cycle1 )
        {
            evolve_one_time_in_cycle1();
        }

        while(    state == State::Cycle2
               || state == State::LastCycle2 )
        {
            evolve_one_time_in_cycle2();
        }
    }
}

void ActiveContour::evolve_n_iterations(int n_iter)
{
    if( n_iter < 1 )
    {
        n_iter = 1;
    }

    for( int i = 0;
         i < n_iter && state != State::Stopped;
         i++ )
    {
        if ( state == State::Cycle1 )
        {
            evolve_one_time_in_cycle1();
        }
        else if (    state == State::Cycle2
                  || state == State::LastCycle2 )
        {
            evolve_one_time_in_cycle2();
        }
    }
}

void ActiveContour::evolve_one_iteration()
{
    evolve_n_iterations( 1 );
}

void ActiveContour::evolve_n_cycles(int n_cycles)
{
    if ( n_cycles < 1 )
    {
        n_cycles = 1;
    }

    int n_iter = n_cycles * n_iter_by_cycle;

    evolve_n_iterations( n_iter );
}

void ActiveContour::evolve_one_time_in_cycle1()
{
    do_specific_cycle1();

    bool is_outward_moving = evolve_one_way( WayContextConfig::SwitchIn );
    bool is_inward_moving  = evolve_one_way( WayContextConfig::SwitchOut );

    ed.total_iter++;
    ed.cycle_iter++;

    ed.is_moving = is_outward_moving || is_inward_moving;

    check_stopped_state();
    calculate_state_cycle_1();
}

void ActiveContour::evolve_one_time_in_cycle2()
{
    evolve_one_way( WayContextConfig::SwitchIn );
    evolve_one_way( WayContextConfig::SwitchOut );

    ed.total_iter++;
    ed.cycle_iter++;

    check_stopped_state();
    calculate_state_cycle_2();
}

bool ActiveContour::evolve_one_way(WayContextConfig ctx_cfg)
{
    bool is_moving = false;

    ctx.set_context( ctx_cfg );

    auto& scanned  = *ctx.scanned_boundary;
    auto& adjacent = *ctx.adjacent_boundary;

    points_to_append.clear();

    compute_speed( scanned );

    for( std::size_t i = 0; i < scanned.size(); )
    {
        auto& point = scanned[i];

        if( point.get_speed() == ctx.way )
        {
            is_moving = true;

            do_specific_when_switch( point.get_offset(),
                                     ctx.config );

            switch_one_way( point );
        }
        else
        {
            i++;
        }
    }

    scanned.insert( scanned.end(),
                    points_to_append.begin(),
                    points_to_append.end() );

    if( is_moving )
    {
        eliminate_redundant_points( adjacent,
                                    ctx.region_redundant_phi_val );
    }

    return is_moving;
}

void ActiveContour::eliminate_redundant_points(ContourList& boundary,
                                               PhiValue region_value)
{
    for( std::size_t i = 0; i < boundary.size(); )
    {
        auto& point = boundary[i];

        if( cd.is_redundant(point) )
        {
            cd.get_phi()[ point.get_offset() ] = region_value;
            point = boundary.back();
            boundary.pop_back();
        }
        else
        {
            i++;
        }
    }
}

void ActiveContour::compute_speed(ContourList& boundary)
{
    if( state == State::Cycle1 )
    {
        compute_external_speed_Fd( boundary );
    }
    else if( state == State::Cycle2 ||
             state == State::LastCycle2 )
    {
        compute_internal_speed_Fint( boundary );
    }
}

void ActiveContour::compute_external_speed_Fd(ContourList& boundary)
{
    for( auto& point : boundary )
    {
        compute_external_speed_Fd( point );
    }
}

// input integer is an offset
void ActiveContour::compute_external_speed_Fd(ContourPoint& point)
{
    // this class should never be instantiated
    // reimplement a better and data-dependent speed function in a child class
    point.set_speed( SpeedValue::GoInward );
}

void ActiveContour::compute_internal_speed_Fint(ContourList& boundary)
{
    for( auto& point : boundary )
    {
        compute_internal_speed_Fint( point );
    }
}

void ActiveContour::compute_internal_speed_Fint(ContourPoint& point)
{
    assert( kernel.weights.size() == kernel.offsets.size() );
    assert( kernel.size == config.kernel_length * config.kernel_length );

    const auto& phi = cd.get_phi();
    const int width  = phi.get_width();
    const int height = phi.get_height();

    const int radius = kernel.radius;
    const int offset = point.get_offset();
    const auto phi_center  = phi[offset];
    const int center_sign = phiSign( phi_center );

    int Fint = 0;

    const int x = point.get_x();
    const int y = offset / width;

    // --- Chemin rapide : entièrement dans l'image ---
    if (   x >= radius
        && x <  width  - radius
        && y >= radius
        && y <  height - radius)
    {
        const int* w = kernel.weights.data();
        const int* o = kernel.offsets.data();

        for (int i = 0; i < kernel.size; ++i)
        {
            Fint -=  w[i] * phiSign( phi[offset + o[i]] );
        }
    }
    // --- Chemin lent : bords ---
    else
    {
        const int* w = kernel.weights.data();
        const int* o = kernel.offsets.data();

        for (int i = 0; i < kernel.size; ++i)
        {
            const int n = offset + o[i];

            if (n >= 0 && n < width * height)
                Fint -= w[i] * phiSign( phi[n] );
            else
                Fint -= w[i] * center_sign;
        }
    }

    point.set_speed( get_discrete_speed(Fint) );
}

void ActiveContour::check_stopped_state()
{
    if( cd.get_l_out().empty() || cd.get_l_in().empty() )
    {
        state = State::Stopped;
        stopping_status = StoppingStatus::EmptyList;
    }
    else if( ed.total_iter >= ed.total_iter_max )
    {
        state = State::Stopped;
        stopping_status = StoppingStatus::MaxIteration;
    }
}

void ActiveContour::calculate_state_cycle_1()
{
    if( state == State::Cycle1 )
    {
        if( !ed.is_moving )
        {
            if( config.is_cycle2 )
            {
                ed.cycle_iter = 0;
                state = State::LastCycle2;
            }
            else
            {
                state = State::Stopped;
                stopping_status = StoppingStatus::ListsStopped;
            }
        }
        else if( ed.cycle_iter >= config.Na &&
                 config.is_cycle2 )
        {
            ed.cycle_iter = 0;
            state = State::Cycle2;
        }
    }
}

void ActiveContour::calculate_state_cycle_2()
{
    if( state == State::Cycle2 ||
        state == State::LastCycle2 )
    {
        // if at the end of one cycle 2
        if( ed.cycle_iter >= config.Ns )
        {
            if( state == State::LastCycle2 )
            {
                state = State::Stopped;
                stopping_status = StoppingStatus::ListsStopped;
            }
            else if( is_cycle2_condition &&
                     state == State::Cycle2 )
            {
                float diff_iter = float(ed.total_iter - ed.previous_total_iter);

                const int w = cd.get_phi().get_width();
                const int h = cd.get_phi().get_height();
                const int diagonal = Shape::get_grid_diagonal(w, h);

                // normalize in function of l_outshape/phi size
                // to handle different images sizes.
                if( diff_iter >= diagonal / 20.f )
                {
                    ed.l_out_shape.clear();

                    for( const auto& point : cd.get_l_out() )
                    {
                        Point_i p = from_ContourPoint( point,
                                                       cd.get_phi().get_width() );

                        ed.l_out_shape.push_back( p );
                    }
                    ed.l_out_shape.calculate_centroid();

                    calculate_shapes_intersection();

                    HausdorffDistance hd( ed.l_out_shape,
                                          ed.previous_shape,
                                          ed.intersection );

                    float size_factor = 100.f / diagonal;
                    ed.hausdorff_quantile = size_factor * hd.get_hausdorff_quantile(80);
                    ed.centroids_distance = size_factor * hd.get_centroids_distance();
                    float delta_quantile = ed.previous_quantile - ed.hausdorff_quantile;

                    if ( ( ed.centroids_distance < 1.f &&
                           ed.hausdorff_quantile < 1.f ) ||
                         ( ed.centroids_distance < 1.f &&
                           ed.hausdorff_quantile < 2.f &&
                           delta_quantile < 0.f ) )
                    {
                        state = State::Stopped;
                        stopping_status = StoppingStatus::Hausdorff;
                    }

                    // swap the shapes in constant time, in complexity O(1)
                    // to prepare data for the next cycle 2 stopping condition iteration.
                    ed.l_out_shape.swap(ed.previous_shape);
                    ed.previous_total_iter = ed.total_iter;
                    ed.previous_quantile = ed.hausdorff_quantile;
                }
            }

            if ( state != State::Stopped )
            {
                ed.cycle_iter = 0;
                state = State::Cycle1;
            }
        }
    }
}

Point_i ActiveContour::from_ContourPoint(const ContourPoint& point,
                                         int grid_width)
{
    Point_i p;

    p.x = point.get_x();
    p.y = point.get_offset() / grid_width;

    return p;
}

void ActiveContour::calculate_shapes_intersection()
{
    ed.intersection.clear();

    for( const auto& point : ed.previous_shape.get_points() )
    {
        // if a point of ed.previous_shape ∈ ed.l_out_shape
        if( cd.get_phi()(point.x, point.y) == PhiValue::ExteriorBoundary )
        {
            ed.intersection.insert( point );
        }
    }
}

void ActiveContour::reinitialize()
{
    cd.check_lists();

    state = State::Cycle1;
    stopping_status = StoppingStatus::None;
    is_cycle2_condition = false;

    ed.reinitialize();
}

}
