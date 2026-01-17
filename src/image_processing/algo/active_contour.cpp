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
    state_( PhaseState::Cycle1 ),
    stopping_status_( StoppingStatus::None ),
    ctx_in_(BoundarySwitchContext::make_switch_in(cd_)),
    ctx_out_(BoundarySwitchContext::make_switch_out(cd_)),
    ctx_(&ctx_in_),
    ed_( cd_ ),
    is_cycle2_condition_( true )
{
    build_internal_kernel_offsets();

    points_to_append_.reserve( cd_.preallocation_size() );

    if ( config_.is_cycle2 )
    {
        steps_per_cycle_ = config_.Na + config_.Ns;
    }
    else
    {
        steps_per_cycle_ = config_.Na;
    }
}

ActiveContour::ActiveContour(ContourData&& initial_contour,
                             const AcConfig& config1)
    : config_(config1),
    cd_( std::move(initial_contour) ),
    state_( PhaseState::Cycle1 ),
    stopping_status_( StoppingStatus::None ),
    ctx_in_(BoundarySwitchContext::make_switch_in(cd_)),
    ctx_out_(BoundarySwitchContext::make_switch_out(cd_)),
    ctx_(&ctx_in_),
    ed_( cd_ ),
    is_cycle2_condition_(true)
{
    build_internal_kernel_offsets();

    points_to_append_.reserve( cd_.preallocation_size() );

    if ( config_.is_cycle2 )
    {
        steps_per_cycle_ = config_.Na + config_.Ns;
    }
    else
    {
        steps_per_cycle_ = config_.Na;
    }
}

void ActiveContour::converge()
{
    // Fast Two Cycle algorithm

    while( state_ != PhaseState::Stopped )
    {
        while( state_ == PhaseState::Cycle1 )
        {
            step_cycle1();
        }

        while(    state_ == PhaseState::Cycle2
               || state_ == PhaseState::FinalCycle2 )
        {
            step_cycle2();
        }
    }
}

void ActiveContour::run_steps(int n_steps)
{
    if( n_steps < 1 )
    {
        n_steps = 1;
    }

    for( int i = 0;
         i < n_steps && state_ != PhaseState::Stopped;
         i++ )
    {
        if ( state_ == PhaseState::Cycle1 )
        {
            step_cycle1();
        }
        else if (    state_ == PhaseState::Cycle2
                  || state_ == PhaseState::FinalCycle2 )
        {
            step_cycle2();
        }
    }
}

void ActiveContour::step()
{
    run_steps( 1 );
}

void ActiveContour::run_cycles(int n_cycles)
{
    if ( n_cycles < 1 )
    {
        n_cycles = 1;
    }

    int n_steps = n_cycles * steps_per_cycle_;

    run_steps( n_steps );
}

void ActiveContour::step_cycle1()
{
    do_specific_cycle1();

    bool is_outward_moving = directional_substep( BoundarySwitch::In );
    bool is_inward_moving  = directional_substep( BoundarySwitch::Out );

    ed_.step_count++;
    ed_.phase_step_count++;

    ed_.is_moving = is_outward_moving || is_inward_moving;

    check_stopping_condition();
    check_state_step1();
}

void ActiveContour::step_cycle2()
{
    directional_substep( BoundarySwitch::In );
    directional_substep( BoundarySwitch::Out );

    ed_.step_count++;
    ed_.phase_step_count++;

    check_stopping_condition();
    update_state_cycle2();
}

bool ActiveContour::stopped() const
{
    return state_ == PhaseState::Stopped;
}

bool ActiveContour::converged() const
{
    return    stopping_status_ == StoppingStatus::ListsStopped
           || stopping_status_ == StoppingStatus::Hausdorff;
}

void ActiveContour::select_context(BoundarySwitch ctx_choice)
{
    if( ctx_choice == BoundarySwitch::In )
    {
        ctx_ = &ctx_in_;
    }
    else if ( ctx_choice == BoundarySwitch::Out )
    {
        ctx_ = &ctx_out_;
    }
}

bool ActiveContour::directional_substep(BoundarySwitch ctx_choice)
{
    bool is_moving = false;

    select_context( ctx_choice );
    const auto& ctx = context();

    auto& scanned  = ctx.scanned_boundary;
    auto& adjacent = ctx.adjacent_boundary;

    points_to_append_.clear();

    compute_speed( scanned );

    for( std::size_t i = 0; i < scanned.size(); )
    {
        auto& point = scanned[i];

        if( point.speed() == ctx.target_direction )
        {
            is_moving = true;

            do_specific_when_switch( point.offset(),
                                     ctx_choice );

            switch_boundary_point( point );
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
                                    ctx.region_redundant_phi_val );
    }

    return is_moving;
}

void ActiveContour::switch_boundary_point(ContourPoint& point)
{
    int offset = point.offset();
    int x = point.x();
    int w = cd_.phi().width();
    int h = cd_.phi().height();
    int last_row_offset = w * (h - 1);

    // Voisins horizontaux
    if (x > 0)
    {
        int left_offset = offset - 1;
        add_region_neighbor(left_offset, x-1);
    }

    if (x < w - 1)
    {
        int right_offset = offset + 1;
        add_region_neighbor(right_offset, x+1);
    }

    // Voisins verticaux
    if (offset >= w) // Pas dans la première ligne
    {
        int up_offset = offset - w;
        add_region_neighbor(up_offset, x);
    }

    if (offset < last_row_offset) // Pas dans la dernière ligne
    {
        int down_offset = offset + w;
        add_region_neighbor(down_offset, x);
    }

#ifdef ALGO_8_CONNEXITY
    // Diagonaux supérieurs
    if (x > 0 && offset >= w)
    {
        int up_left_offset = offset - w - 1;
        add_region_neighbor(up_left_offset, x-1);
    }

    if (x < w - 1 && offset >= w)
    {
        int up_right_offset = offset - w + 1;
        add_region_neighbor(up_right_offset, x+1);
    }

    // Diagonaux inférieurs
    if (x > 0 && offset < last_row_offset)
    {
        int down_left_offset = offset + w - 1;
        add_region_neighbor(down_left_offset, x-1);
    }

    if (x < w - 1 && offset < last_row_offset)
    {
        int down_right_offset = offset + w + 1;
        add_region_neighbor(down_right_offset, x+1);
    }
#endif

    const auto& ctx = context();

    // change the phi value of the current point
    // according to the phi value of the adjacent list
    cd_.phi()[ offset ] = ctx.adjacent_phi_val;

    // switch the current point to the adjacent boundary list
    ctx.adjacent_boundary.emplace_back( offset, x );

    point = ctx.scanned_boundary.back();
    ctx.scanned_boundary.pop_back();
}

void ActiveContour::add_region_neighbor(int neighbor_offset,
                                        int neighbor_x)
{
    const auto& ctx = context();

    // if a neighbor ∈ one region
    if( cd_.phi()[ neighbor_offset ] == ctx.neighbor_region_phi_val )
    {
        cd_.phi()[ neighbor_offset ] = ctx.neighbor_boundary_phi_val;

        // neighbor ∈ region ==> ∈ neighbor list
        points_to_append_.emplace_back( neighbor_offset, neighbor_x );
    }
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
    if( state_ == PhaseState::Cycle1 )
    {
        compute_external_speed_Fd( boundary );
    }
    else if( state_ == PhaseState::Cycle2 ||
             state_ == PhaseState::FinalCycle2 )
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

void ActiveContour::build_internal_kernel_offsets()
{
    internal_kernel_offsets_.clear();

    const int r = config_.disk_radius;
    const int w = cd_.phi().width();

    for ( int dy = -r; dy <= r; ++dy )
    {
        for ( int dx = -r; dx <= r; ++dx )
        {
            if ( dx*dx + dy*dy <= r*r )
                internal_kernel_offsets_.push_back( dy * w + dx );
        }
    }
}

void ActiveContour::compute_internal_speed_Fint(ContourPoint& point)
{
    // Internal speed Fint is computed using a local majority vote
    // on the interior/exterior labeling of neighboring pixels.
    // This is equivalent to a smoothed Heaviside convolution
    // described in the reference paper, but implemented in a
    // simpler and more robust discrete form.

    // Note: the direction is intentionally inverted.
    // If the neighborhood is mostly inside, the boundary locally
    // protrudes outward and must be pushed outward to smooth curvature.

    const int base = point.offset();

    int neighbor_offset;

    int inside = 0;
    int outside = 0;

    for (int delta : internal_kernel_offsets_)
    {
        neighbor_offset = base + delta;

        if ( cd_.phi().valid( neighbor_offset ) )
        {
            PhiValue v = cd_.phi()[ neighbor_offset ];

            if ( phi_value::isInside(v) )
                ++inside;
            else
                ++outside;
        }
    }

    // intentionally inverted, here.
    if (inside > outside)
        point.set_speed(SpeedValue::GoOutward);
    else if (outside > inside)
        point.set_speed(SpeedValue::GoInward);
    else
        point.set_speed(SpeedValue::NoMove);
}

void ActiveContour::stop(StoppingStatus reason)
{
    state_ = PhaseState::Stopped;
    stopping_status_ = reason;
}

void ActiveContour::check_stopping_condition()
{
    if( cd_.l_out().empty() || cd_.l_in().empty() )
    {
        stop( StoppingStatus::EmptyList );
    }
    else if( ed_.step_count >= ed_.max_step_count )
    {
        stop( StoppingStatus::MaxIteration );
    }
}

void ActiveContour::check_state_step1()
{
    if( state_ == PhaseState::Cycle1 )
    {
        if( !ed_.is_moving )
        {
            if( config_.is_cycle2 )
            {
                ed_.phase_step_count = 0;
                state_ = PhaseState::FinalCycle2;
            }
            else
            {
                stop( StoppingStatus::ListsStopped );
            }
        }
        else if( ed_.phase_step_count >= config_.Na &&
                 config_.is_cycle2 )
        {
            ed_.phase_step_count = 0;
            state_ = PhaseState::Cycle2;
        }
    }
}

void ActiveContour::update_state_cycle2()
{
    if( state_ == PhaseState::Cycle2 ||
        state_ == PhaseState::FinalCycle2 )
    {
        // if at the end of one cycle 2
        if( ed_.phase_step_count >= config_.Ns )
        {
            if( state_ == PhaseState::FinalCycle2 )
            {
                stop( StoppingStatus::ListsStopped );
            }
            else if( is_cycle2_condition_ &&
                     state_ == PhaseState::Cycle2 )
            {
                float step_delta = float(ed_.step_count - ed_.previous_step_count);

                const int w = cd_.phi().width();
                const int h = cd_.phi().height();
                const int diagonal = Shape::get_grid_diagonal(w, h);

                // normalize in function of l_outshape/phi size
                // to handle different images sizes.
                if( step_delta >= diagonal / 20.f )
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
                        stop( StoppingStatus::Hausdorff );
                    }

                    // swap the shapes in constant time, in complexity O(1)
                    // to prepare data for the next cycle 2 stopping condition iteration.
                    ed_.l_out_shape.swap(ed_.previous_shape);
                    ed_.previous_step_count = ed_.step_count;
                    ed_.previous_quantile = ed_.hausdorff_quantile;
                }
            }

            if ( state_ != PhaseState::Stopped )
            {
                ed_.phase_step_count = 0;
                state_ = PhaseState::Cycle1;
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

    state_ = PhaseState::Cycle1;
    stopping_status_ = StoppingStatus::None;
    is_cycle2_condition_ = false;

    ed_.reinitialize();
}

}
