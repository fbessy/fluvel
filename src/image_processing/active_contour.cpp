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

#include "active_contour.hpp"
#include "hausdorff_distance.hpp"


namespace ofeli_ip
{

// Definitions
ActiveContour::ActiveContour(const ContourData& initial_contour,
                             const AcConfig& config1)
    : config(config1),
    gaussian_kernel(config.kernel_length, config.sigma),
    cd( initial_contour ),
    state( State::CYCLE_1 ),
    stopping_status( StoppingStatus::NONE ),
    ctx( cd ),
    ed( cd ),
    is_cycle2_condition( true )
{
    speed.reserve( INITIAL_SPEED_ARRAY_ALLOC_SIZE );

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
    gaussian_kernel(config.kernel_length, config.sigma),
    cd( std::move(initial_contour) ),
    state( State::CYCLE_1 ),
    stopping_status( StoppingStatus::NONE ),
    ctx( cd ),
    ed( cd ),
    is_cycle2_condition(true)
{
    speed.reserve( INITIAL_SPEED_ARRAY_ALLOC_SIZE );

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

    while( state != State::STOPPED )
    {
        while( state == State::CYCLE_1 )
        {
            evolve_one_time_in_cycle1();
        }

        while(    state == State::CYCLE_2
               || state == State::LAST_CYCLE_2 )
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
         i < n_iter && state != State::STOPPED;
         i++ )
    {
        if ( state == CYCLE_1 )
        {
            evolve_one_time_in_cycle1();
        }
        else if (    state == State::CYCLE_2
                  || state == State::LAST_CYCLE_2 )
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

    bool is_outward_moving = evolve_one_way( WayContextConfig::SWITCH_IN );
    bool is_inward_moving  = evolve_one_way( WayContextConfig::SWITCH_OUT );

    ed.total_iter++;
    ed.cycle_iter++;

    ed.is_moving = is_outward_moving || is_inward_moving;

    check_stopped_state();
    calculate_state_cycle_1();
}

void ActiveContour::evolve_one_time_in_cycle2()
{
    evolve_one_way( WayContextConfig::SWITCH_IN );
    evolve_one_way( WayContextConfig::SWITCH_OUT );

    ed.total_iter++;
    ed.cycle_iter++;

    check_stopped_state();
    calculate_state_cycle_2();
}

bool ActiveContour::evolve_one_way(WayContextConfig ctx_cfg)
{
    bool is_moving = false;

    ctx.set_context( ctx_cfg );

    compute_speed( *ctx.scanned_boundary );

    int speed_idx = 0;

    for( auto point = ctx.scanned_boundary->begin();
         point != ctx.scanned_boundary->end();
         speed_idx++ )
    {
        if( speed[ speed_idx ] == ctx.way )
        {
            is_moving = true;

            do_specific_when_switch( *point, ctx.config );

            point = switch_one_way( point );
        }
        else
        {
            ++point;
        }
    }

    if( is_moving )
    {
        eliminate_redundant_points( *ctx.adjacent_boundary,
                                    ctx.region_redundant_phi_val );
    }

    return is_moving;
}

void ActiveContour::eliminate_redundant_points(List_i& boundary,
                                               PhiValue region_value)
{
    for( auto point = boundary.begin(); point != boundary.end();   )
    {
        if( cd.is_boundary_redundant( *point ) )
        {
            cd.get_phi()[ *point ] = region_value;
            point = boundary.erase( point ); // it returns the following point
        }
        else
        {
            ++point;
        }
    }
}

void ActiveContour::compute_speed(const List_i& boundary)
{
    if( state == State::CYCLE_1 )
    {
        compute_external_speed_Fd( boundary );
    }
    else if( state == State::CYCLE_2 ||
             state == State::LAST_CYCLE_2 )
    {
        compute_internal_speed_Fint( boundary );
    }
}

void ActiveContour::compute_external_speed_Fd(const List_i& boundary)
{
    speed.resize( 0u );

    for( auto point = boundary.cbegin(); point != boundary.cend(); ++point )
    {
        speed.push_back( compute_external_speed_Fd( *point ) );
    }
}

// input integer is an offset
SpeedValue ActiveContour::compute_external_speed_Fd(int)
{
    // this class should never be instantiated
    // reimplement a better and data-dependent speed function in a child class

    return GO_INWARD;
}

void ActiveContour::compute_internal_speed_Fint(const List_i& boundary)
{
    speed.resize( 0u );

    for( auto point = boundary.cbegin(); point != boundary.cend(); ++point )
    {
        speed.push_back( compute_internal_speed_Fint( *point ) );
    }
}

signed char ActiveContour::compute_internal_speed_Fint(int offset)
{
    int x, y;
    cd.get_phi().get_position(offset,x,y); // x and y passed by reference

    const int kernel_radius = (gaussian_kernel.get_width() - 1) / 2;
    int Fint = 0;

    // if not in the image's border, no neighbors' tests
    if(    x-kernel_radius >= 0
        && x+kernel_radius < cd.get_phi().get_width()
        && y-kernel_radius >= 0
        && y+kernel_radius < cd.get_phi().get_height()
      )
    {
        for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
        {
            for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
            {
                Fint += int( gaussian_kernel(kernel_radius+dx,kernel_radius+dy) )
                        * get_sign_opposite( cd.get_phi()(x+dx,y+dy) );
            }
        }
    }
    // if in the border of the image, tests of neighbors
    else
    {
        for( int dy = -kernel_radius; dy <= kernel_radius; dy++ )
        {
            for( int dx = -kernel_radius; dx <= kernel_radius; dx++ )
            {
                if(    x+dx >= 0
                    && x+dx < cd.get_phi().get_width()
                    && y+dy >= 0
                    && y+dy < cd.get_phi().get_height()
                  )
                {
                    Fint += int( gaussian_kernel(kernel_radius+dx,kernel_radius+dy) )
                            * get_sign_opposite( cd.get_phi()(x+dx,y+dy) );
                }
                else
                {
                    Fint += int( gaussian_kernel(kernel_radius+dx,kernel_radius+dy) )
                            * get_sign_opposite( cd.get_phi()[offset] );
                }
            }
        }
    }

    return get_discrete_speed( Fint );
}

void ActiveContour::check_stopped_state()
{
    if( cd.get_l_out().empty() || cd.get_l_in().empty() )
    {
        state = State::STOPPED;
        stopping_status = StoppingStatus::EMPTY_LIST;
    }
    else if( ed.total_iter >= ed.total_iter_max )
    {
        state = State::STOPPED;
        stopping_status = StoppingStatus::MAX_ITERATION;
    }
}

void ActiveContour::calculate_state_cycle_1()
{
    if( state == State::CYCLE_1 )
    {
        if( !ed.is_moving )
        {
            if( config.is_cycle2 )
            {
                ed.cycle_iter = 0;
                state = State::LAST_CYCLE_2;
            }
            else
            {
                state = State::STOPPED;
                stopping_status = StoppingStatus::LISTS_STOPPED;
            }
        }
        else if( ed.cycle_iter >= config.Na &&
                 config.is_cycle2 )
        {
            ed.cycle_iter = 0;
            state = State::CYCLE_2;
        }
    }
}

void ActiveContour::calculate_state_cycle_2()
{
    if( state == State::CYCLE_2 ||
        state == State::LAST_CYCLE_2 )
    {
        // if at the end of one cycle 2
        if( ed.cycle_iter >= config.Ns )
        {
            if( state == State::LAST_CYCLE_2 )
            {
                state = State::STOPPED;
                stopping_status = StoppingStatus::LISTS_STOPPED;
            }
            else if( is_cycle2_condition &&
                     state == State::CYCLE_2 )
            {
                float diff_iter = float(ed.total_iter - ed.previous_total_iter);

                // normalize in function of l_outshape/phi size
                // to handle different images sizes.
                if( diff_iter >= ed.l_out_shape.get_grid_diagonal() / 20.f )
                {
                    // retrieve a std::vector from a std::list
                    // (needed to shuffle points for haudorff distance
                    // efficient computation)
                    // and compute l_out centroid
                    ed.l_out_shape.transform( cd.get_l_out() );

                    calculate_shapes_intersection();

                    HausdorffDistance hd( ed.l_out_shape,
                                          ed.previous_shape,
                                          ed.intersection );

                    float size_factor = 100.f / hd.get_grid_diagonal();
                    ed.hausdorff_quantile = size_factor * hd.get_hausdorff_quantile(80);
                    ed.centroids_distance = size_factor * hd.get_centroids_distance();
                    float delta_quantile = ed.previous_quantile - ed.hausdorff_quantile;

                    if ( ( ed.centroids_distance < 1.f &&
                           ed.hausdorff_quantile < 1.f ) ||
                         ( ed.centroids_distance < 1.f &&
                           ed.hausdorff_quantile < 2.f &&
                           delta_quantile < 0.f ) )
                    {
                        state = State::STOPPED;
                        stopping_status = StoppingStatus::HAUSDORFF;
                    }

                    // swap the shapes in constant time, in complexity O(1)
                    // to prepare data for the next cycle 2 stopping condition iteration.
                    ed.l_out_shape.swap(ed.previous_shape);
                    ed.previous_total_iter = ed.total_iter;
                    ed.previous_quantile = ed.hausdorff_quantile;
                }
            }

            if ( state != State::STOPPED )
            {
                ed.cycle_iter = 0;
                state = State::CYCLE_1;
            }
        }
    }
}

void ActiveContour::calculate_shapes_intersection()
{
    ed.intersection.clear();

    for( auto point = ed.previous_shape.get_points().cbegin();
         point != ed.previous_shape.get_points().cend();
         ++point )
    {
        // if a point of ed.previous_shape ∈ ed.l_out_shape
        if( cd.get_phi()[ *point ] == PhiValue::EXTERIOR_BOUNDARY )
        {
            ed.intersection.insert( *point );
        }
    }
}

void ActiveContour::reinitialize()
{
    cd.check_lists();

    state = State::CYCLE_1;
    stopping_status = StoppingStatus::NONE;
    is_cycle2_condition = false;

    ed.reinitialize();
}

}
