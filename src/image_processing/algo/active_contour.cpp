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
    : config_(config1),
    cd_( initial_contour ),
    state_( State::Cycle1 ),
    stopping_status_( StoppingStatus::None ),
    ctx_( cd_ ),
    ed_( cd_ ),
    is_cycle2_condition_( true )
{
    kernel_ = build_gaussian_kernel_linear(config_.kernel_length,
                                           config_.sigma,
                                           cd_.phi().width());

    points_to_append_.reserve( cd_.preallocation_size() );

    if ( config_.is_cycle2 )
    {
        n_iter_by_cycle_ = config_.Na + config_.Ns;
    }
    else
    {
        n_iter_by_cycle_ = config_.Na;
    }
}

ActiveContour::ActiveContour(ContourData&& initial_contour,
                             const AcConfig& config1)
    : config_(config1),
    cd_( std::move(initial_contour) ),
    state_( State::Cycle1 ),
    stopping_status_( StoppingStatus::None ),
    ctx_( cd_ ),
    ed_( cd_ ),
    is_cycle2_condition_(true)
{
    kernel_ = build_gaussian_kernel_linear(config_.kernel_length,
                                           config_.sigma,
                                           cd_.phi().width());

    points_to_append_.reserve( cd_.preallocation_size() );

    if ( config_.is_cycle2 )
    {
        n_iter_by_cycle_ = config_.Na + config_.Ns;
    }
    else
    {
        n_iter_by_cycle_ = config_.Na;
    }
}

void ActiveContour::evolve()
{
    // Fast Two Cycle algorithm

    while( state_ != State::Stopped )
    {
        while( state_ == State::Cycle1 )
        {
            evolve_one_time_in_cycle1();
        }

        while(    state_ == State::Cycle2
               || state_ == State::LastCycle2 )
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
         i < n_iter && state_ != State::Stopped;
         i++ )
    {
        if ( state_ == State::Cycle1 )
        {
            evolve_one_time_in_cycle1();
        }
        else if (    state_ == State::Cycle2
                  || state_ == State::LastCycle2 )
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

    int n_iter = n_cycles * n_iter_by_cycle_;

    evolve_n_iterations( n_iter );
}

void ActiveContour::evolve_one_time_in_cycle1()
{
    do_specific_cycle1();

    bool is_outward_moving = evolve_one_way( WayContextConfig::SwitchIn );
    bool is_inward_moving  = evolve_one_way( WayContextConfig::SwitchOut );

    ed_.total_iter++;
    ed_.cycle_iter++;

    ed_.is_moving = is_outward_moving || is_inward_moving;

    check_stopped_state();
    calculate_state_cycle_1();
}

void ActiveContour::evolve_one_time_in_cycle2()
{
    evolve_one_way( WayContextConfig::SwitchIn );
    evolve_one_way( WayContextConfig::SwitchOut );

    ed_.total_iter++;
    ed_.cycle_iter++;

    check_stopped_state();
    calculate_state_cycle_2();
}

bool ActiveContour::evolve_one_way(WayContextConfig ctx_cfg)
{
    bool is_moving = false;

    ctx_.set_context( ctx_cfg );

    auto& scanned  = *ctx_.scanned_boundary;
    auto& adjacent = *ctx_.adjacent_boundary;

    points_to_append_.clear();

    compute_speed( scanned );

    for( std::size_t i = 0; i < scanned.size(); )
    {
        auto& point = scanned[i];

        if( point.speed() == ctx_.way )
        {
            is_moving = true;

            do_specific_when_switch( point.offset(),
                                     ctx_.config );

            switch_one_way( point );
        }
        else
        {
            i++;
        }
    }

    scanned.insert( scanned.end(),
                    points_to_append_.begin(),
                    points_to_append_.end() );

    if( is_moving )
    {
        eliminate_redundant_points( adjacent,
                                    ctx_.region_redundant_phi_val );
    }

    return is_moving;
}

void ActiveContour::eliminate_redundant_points(ContourList& boundary,
                                               PhiValue region_value)
{
    for( std::size_t i = 0; i < boundary.size(); )
    {
        auto& point = boundary[i];

        if( cd_.is_redundant(point) )
        {
            cd_.phi()[ point.offset() ] = region_value;
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
    if( state_ == State::Cycle1 )
    {
        compute_external_speed_Fd( boundary );
    }
    else if( state_ == State::Cycle2 ||
             state_ == State::LastCycle2 )
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
    assert( kernel_.weights.size() == kernel_.offsets.size() );
    assert( kernel_.size == config_.kernel_length * config_.kernel_length );

    const auto& phi = cd_.phi();
    const int width  = phi.width();
    const int height = phi.height();

    const int radius = kernel_.radius;
    const int offset = point.offset();
    const auto phi_center  = phi[offset];
    const int center_sign = phiSign( phi_center );

    int Fint = 0;

    const int x = point.x();
    const int y = offset / width;

    // --- Chemin rapide : entièrement dans l'image ---
    if (   x >= radius
        && x <  width  - radius
        && y >= radius
        && y <  height - radius)
    {
        const int* w = kernel_.weights.data();
        const int* o = kernel_.offsets.data();

        for (int i = 0; i < kernel_.size; ++i)
        {
            Fint -=  w[i] * phiSign( phi[offset + o[i]] );
        }
    }
    // --- Chemin lent : bords ---
    else
    {
        const int* w = kernel_.weights.data();
        const int* o = kernel_.offsets.data();

        for (int i = 0; i < kernel_.size; ++i)
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
    if( cd_.l_out().empty() || cd_.l_in().empty() )
    {
        state_ = State::Stopped;
        stopping_status_ = StoppingStatus::EmptyList;
    }
    else if( ed_.total_iter >= ed_.total_iter_max )
    {
        state_ = State::Stopped;
        stopping_status_ = StoppingStatus::MaxIteration;
    }
}

void ActiveContour::calculate_state_cycle_1()
{
    if( state_ == State::Cycle1 )
    {
        if( !ed_.is_moving )
        {
            if( config_.is_cycle2 )
            {
                ed_.cycle_iter = 0;
                state_ = State::LastCycle2;
            }
            else
            {
                state_ = State::Stopped;
                stopping_status_ = StoppingStatus::ListsStopped;
            }
        }
        else if( ed_.cycle_iter >= config_.Na &&
                 config_.is_cycle2 )
        {
            ed_.cycle_iter = 0;
            state_ = State::Cycle2;
        }
    }
}

void ActiveContour::calculate_state_cycle_2()
{
    if( state_ == State::Cycle2 ||
        state_ == State::LastCycle2 )
    {
        // if at the end of one cycle 2
        if( ed_.cycle_iter >= config_.Ns )
        {
            if( state_ == State::LastCycle2 )
            {
                state_ = State::Stopped;
                stopping_status_ = StoppingStatus::ListsStopped;
            }
            else if( is_cycle2_condition_ &&
                     state_ == State::Cycle2 )
            {
                float diff_iter = float(ed_.total_iter - ed_.previous_total_iter);

                const int w = cd_.phi().width();
                const int h = cd_.phi().height();
                const int diagonal = Shape::get_grid_diagonal(w, h);

                // normalize in function of l_outshape/phi size
                // to handle different images sizes.
                if( diff_iter >= diagonal / 20.f )
                {
                    ed_.l_out_shape.clear();

                    for( const auto& point : cd_.l_out() )
                    {
                        Point2D_i p = from_ContourPoint( point,
                                                         cd_.phi().width() );

                        ed_.l_out_shape.push_back( p );
                    }
                    ed_.l_out_shape.calculate_centroid();

                    calculate_shapes_intersection();

                    HausdorffDistance hd( ed_.l_out_shape,
                                          ed_.previous_shape,
                                          ed_.intersection );

                    float size_factor = 100.f / diagonal;
                    ed_.hausdorff_quantile = size_factor * hd.get_hausdorff_quantile(80);
                    ed_.centroids_distance = size_factor * hd.get_centroids_distance();
                    float delta_quantile = ed_.previous_quantile - ed_.hausdorff_quantile;

                    if ( ( ed_.centroids_distance < 1.f &&
                           ed_.hausdorff_quantile < 1.f ) ||
                         ( ed_.centroids_distance < 1.f &&
                           ed_.hausdorff_quantile < 2.f &&
                           delta_quantile < 0.f ) )
                    {
                        state_ = State::Stopped;
                        stopping_status_ = StoppingStatus::Hausdorff;
                    }

                    // swap the shapes in constant time, in complexity O(1)
                    // to prepare data for the next cycle 2 stopping condition iteration.
                    ed_.l_out_shape.swap(ed_.previous_shape);
                    ed_.previous_total_iter = ed_.total_iter;
                    ed_.previous_quantile = ed_.hausdorff_quantile;
                }
            }

            if ( state_ != State::Stopped )
            {
                ed_.cycle_iter = 0;
                state_ = State::Cycle1;
            }
        }
    }
}

Point2D_i ActiveContour::from_ContourPoint(const ContourPoint& point,
                                           int grid_width)
{
    Point2D_i p;

    p.x = point.x();
    p.y = point.offset() / grid_width;

    return p;
}

void ActiveContour::calculate_shapes_intersection()
{
    ed_.intersection.clear();

    for( const auto& point : ed_.previous_shape.get_points() )
    {
        // if a point of ed.previous_shape ∈ ed.l_out_shape
        if( cd_.phi().at(point.x, point.y) == PhiValue::ExteriorBoundary )
        {
            ed_.intersection.insert( point );
        }
    }
}

void ActiveContour::reinitialize()
{
    cd_.check_lists();

    state_ = State::Cycle1;
    stopping_status_ = StoppingStatus::None;
    is_cycle2_condition_ = false;

    ed_.reinitialize();
}

}
