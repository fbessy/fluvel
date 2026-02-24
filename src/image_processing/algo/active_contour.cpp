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

#include <cstddef>
#include <cstdlib>     // std::abs
#include <cassert>

#include "active_contour.hpp"
#include "hausdorff_distance.hpp"
#include "neighborhood.hpp"


namespace ofeli_ip
{

// Definitions
ActiveContour::ActiveContour(const ContourData& initial_state,
                             const AcConfig& config)
    : cd_(initial_state),
    config_(config),
    internal_kernel_(make_internal_kernel_offsets(config.disk_radius,
                                                          initial_state.phi().width())),
    ctx_in_(BoundarySwitchContext::make_switch_in(cd_)),
    ctx_out_(BoundarySwitchContext::make_switch_out(cd_)),
    ctx_(&ctx_in_),
    state_( PhaseState::Cycle1 ),
    ed_( cd_ )
{
    active_boundary_staging_.reserve( cd_.l_out().capacity() );

    if ( config_.is_cycle2 )
    {
        steps_per_cycle_ = config_.Na + config_.Ns;
    }
    else
    {
        steps_per_cycle_ = config_.Na;
    }
}

ActiveContour::ActiveContour(ContourData&& initial_state,
                             const AcConfig& config)
    : cd_( std::move(initial_state) ),
    config_(config),
    internal_kernel_(make_internal_kernel_offsets(config.disk_radius,
                                                          cd_.phi().width())),
    ctx_in_(BoundarySwitchContext::make_switch_in(cd_)),
    ctx_out_(BoundarySwitchContext::make_switch_out(cd_)),
    ctx_(&ctx_in_),
    state_( PhaseState::Cycle1 ),
    ed_( cd_ )
{
    active_boundary_staging_.reserve( cd_.l_out().capacity() );

    if ( config_.is_cycle2 )
    {
        steps_per_cycle_ = config_.Na + config_.Ns;
    }
    else
    {
        steps_per_cycle_ = config_.Na;
    }
}

void ActiveContour::handle_failure()
{
    if ( !cd_.empty() )
        return;


    ed_.stopping_status = StoppingStatus::EmptyListFailure;
    stop();

    // Recovery mode: the failure stops the current iteration,
    // but contour data are repaired to allow a future restart.
    if ( config_.failure_mode == FailureHandlingMode::RecoverOnFailure )
        cd_.define_from_ellipse();
}

void ActiveContour::enforce_iteration_limit()
{
    assert( state_ == PhaseState::Cycle1 );

    // Condition to handle and avoid infinite loop of the method converge(),
    // The convergence is data-dependent and therefore not guaranteed.
    if( ed_.step_count >= ed_.max_step_count )
    {
        ed_.stopping_status = StoppingStatus::MaxIteration;

        if ( config_.is_cycle2 )
        {
            state_ = PhaseState::FinalCycle2;
        }
        else
        {
            stop();
        }
    }
}

void ActiveContour::converge()
{
    // Fast Two Cycle algorithm

    while( !is_stopped() )
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

    for( int i = 0; i < n_steps; ++i )
    {
        if( is_stopped() )
            return;

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
    assert( state_ == PhaseState::Cycle1 );

    handle_failure();
    if( is_stopped() )
        return;

    enforce_iteration_limit();
    if( is_stopped() )
        return;


    do_specific_cycle1();

    bool is_outward_moving = directional_substep( BoundarySwitch::In );
    bool is_inward_moving  = directional_substep( BoundarySwitch::Out );

    ++ed_.step_count;
    ++ed_.phase_step_count;

    ed_.is_moving = is_outward_moving || is_inward_moving;

    check_state_step1();
}

void ActiveContour::step_cycle2()
{
    assert(    state_ == PhaseState::Cycle2
            || state_ == PhaseState::FinalCycle2 );

    handle_failure();
    if( is_stopped() )
        return;

    directional_substep( BoundarySwitch::In );
    directional_substep( BoundarySwitch::Out );

    ++ed_.step_count;
    ++ed_.phase_step_count;

    update_state_cycle2();
}

void ActiveContour::select_context(BoundarySwitch ctx_choice)
{
    assert( !is_stopped() );

    if( ctx_choice == BoundarySwitch::In )
        ctx_ = &ctx_in_;
    else if ( ctx_choice == BoundarySwitch::Out )
        ctx_ = &ctx_out_;
}

bool ActiveContour::directional_substep(BoundarySwitch ctx_choice)
{
    assert( !is_stopped() );

    bool is_moving = false;

    select_context( ctx_choice );
    const auto& ctx = context();

    auto& active   = ctx.active_boundary;
    auto& adjacent = ctx.adjacent_boundary;

    active_boundary_staging_.clear();

    compute_speed( active );

    for( std::size_t i = 0; i < active.size(); )
    {
        auto& point = active[i];

        if( point.speed_ == ctx.required_speed_sign )
        {
            is_moving = true;

            do_specific_when_switch( point,
                                     ctx_choice );

            switch_boundary_point( point );
        }
        else
        {
            ++i;
        }
    }

    active.insert( active.end(),
                   active_boundary_staging_.begin(),
                   active_boundary_staging_.end() );

    if( is_moving )
    {
        cd_.eliminate_redundant_points( adjacent,
                                        ctx.redundant_to_region_val );
    }

    return is_moving;
}

void ActiveContour::switch_boundary_point(ContourPoint& point)
{
    assert(!is_stopped());

    const int x = point.x();
    const int y = point.y();

    const int w = cd_.phi().width();
    const int h = cd_.phi().height();

    const auto connected = cd_.connectivity();

    // ---------- FAST PATH ----------
    if ( x > 0 && x + 1 < w && y > 0 && y + 1 < h )
    {
        // voisins 4-connectés
        for (const auto& d : kNeighbors4)
        {
            promote_region_to_boundary( x + d.dx, y + d.dy );
        }

        // extension diagonale
        if ( connected == Connectivity::Eight )
        {
            for ( const auto& d : kNeighbors4Diag )
            {
                promote_region_to_boundary( x + d.dx, y + d.dy );
            }
        }
    }
    else
    {
        // ---------- SLOW PATH (bords) ----------
        for ( const auto& d : kNeighbors4 )
        {
            const int nx = x + d.dx;
            const int ny = y + d.dy;

            if (nx < 0 || nx >= w || ny < 0 || ny >= h)
                continue;

            promote_region_to_boundary( nx, ny );
        }

        if ( connected == Connectivity::Eight )
        {
            for ( const auto& d : kNeighbors4Diag )
            {
                const int nx = x + d.dx;
                const int ny = y + d.dy;

                if ( nx < 0 || nx >= w || ny < 0 || ny >= h )
                    continue;

                promote_region_to_boundary( nx, ny );
            }
        }
    }

    const auto& ctx = context();

    // bascule du point courant
    cd_.phi().at(x, y) = ctx.current_to_adjacent_val;

    // passage dans la boundary adjacente
    ctx.adjacent_boundary.emplace_back(x, y);

    point = ctx.active_boundary.back();
    ctx.active_boundary.pop_back();
}

void ActiveContour::promote_region_to_boundary(int nx, int ny)
{
    assert(!is_stopped());

    const auto& ctx = context();

    auto& phi_neighbor = cd_.phi().at(nx, ny);

    // neighbor ∈ region ?
    if (phi_neighbor == ctx.neighbor_from_region_val)
    {
        phi_neighbor = ctx.neighbor_to_boundary_val;

        // neighbor pending to boundary active
        active_boundary_staging_.emplace_back(nx, ny);
    }
}

void ActiveContour::compute_speed(Contour& boundary)
{
    assert( !is_stopped() );

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

void ActiveContour::compute_external_speed_Fd(Contour& boundary)
{
    assert( state_ == PhaseState::Cycle1 );

    for( auto& point : boundary )
    {
        compute_external_speed_Fd( point );
    }
}

// input integer is an offset
void ActiveContour::compute_external_speed_Fd(ContourPoint& point)
{
    assert( state_ == PhaseState::Cycle1 );

    // this class should never be instantiated
    // reimplement a better and data-dependent speed function in a child class
    point.speed_ = SpeedValue::GoInward;
}

void ActiveContour::compute_internal_speed_Fint(Contour& boundary)
{
    assert(    state_ == PhaseState::Cycle2
            || state_ == PhaseState::FinalCycle2 );

    for( auto& point : boundary )
    {
        compute_internal_speed_Fint( point );
    }
}

InternalKernel ActiveContour::make_internal_kernel_offsets(int disk_radius,
                                                           int grid_width)
{
    const int r = disk_radius;
    const int w = grid_width;

    InternalKernel kernel;
    kernel.offsets.reserve( static_cast<std::size_t>((2 * r + 1) * (2 * r + 1)) );

    kernel.support.min_dx =  r;
    kernel.support.max_dx = -r;
    kernel.support.min_dy =  r;
    kernel.support.max_dy = -r;

    for ( int dy = -r; dy <= r; ++dy )
    {
        for ( int dx = -r; dx <= r; ++dx )
        {
            if ( dx*dx + dy*dy <= r*r )
            {
                kernel.offsets.push_back( dy * w + dx );

                kernel.support.min_dx = std::min(kernel.support.min_dx, dx);
                kernel.support.max_dx = std::max(kernel.support.max_dx, dx);
                kernel.support.min_dy = std::min(kernel.support.min_dy, dy);
                kernel.support.max_dy = std::max(kernel.support.max_dy, dy);
            }
        }
    }

    return kernel;
}

void ActiveContour::compute_internal_speed_Fint(ContourPoint& point)
{
    assert(    state_ == PhaseState::Cycle2
            || state_ == PhaseState::FinalCycle2 );

    // Internal speed Fint is computed using a local majority vote
    // on the interior/exterior labeling of neighboring pixels.
    // This is equivalent to a smoothed Heaviside convolution
    // described in the reference paper.

    // Note: the direction is intentionally inverted.
    // If the neighborhood is mostly inside, the boundary locally
    // protrudes outward and must be pushed outward to smooth curvature.

    const int base = cd_.phi().offset( point.x(), point.y() );

    int neighbor_offset;

    int inside = 0;
    int outside = 0;

    if ( internal_kernel_.fully_inside( point.x(), point.y(),
                                        cd_.phi().width(), cd_.phi().height() ) )
    {
        // fast path without all neighbors existence checks

        for (int delta : internal_kernel_.offsets)
        {
            neighbor_offset = base + delta;

            PhiValue v = cd_.phi()[ neighbor_offset ];

            if ( phi_value::isInside(v) )
                ++inside;
            else
                ++outside;
        }
    }
    else
    {
        for (int delta : internal_kernel_.offsets)
        {
            neighbor_offset = base + delta;

            if ( cd_.phi().valid( neighbor_offset ) /* all checks */ )
            {
                PhiValue v = cd_.phi()[ neighbor_offset ];

                if ( phi_value::isInside(v) )
                    ++inside;
                else
                    ++outside;
            }
        }
    }

    // intentionally inverted, here.
    if (inside > outside)
        point.speed_ = SpeedValue::GoOutward;
    else if (outside > inside)
        point.speed_ = SpeedValue::GoInward;
    else
        point.speed_ = SpeedValue::NoMove;
}

void ActiveContour::stop()
{
    assert( state_ != PhaseState::Stopped );

    state_ = PhaseState::Stopped;
}

void ActiveContour::check_state_step1()
{
    assert( state_ == PhaseState::Cycle1 );

    if( !ed_.is_moving )
    {
        ed_.stopping_status = StoppingStatus::ListsConverged;

        if( config_.is_cycle2 )
        {
            ed_.phase_step_count = 0;
            state_ = PhaseState::FinalCycle2;
        }
        else
        {
            stop();
        }
    }
    else if( ed_.phase_step_count >= config_.Na &&
             config_.is_cycle2 )
    {
        ed_.phase_step_count = 0;
        state_ = PhaseState::Cycle2;
    }
}

void ActiveContour::update_state_cycle2()
{
    assert(    state_ == PhaseState::Cycle2
            || state_ == PhaseState::FinalCycle2 );

    // if at the end of one cycle 2
    if( ed_.phase_step_count >= config_.Ns )
    {
        if( state_ == PhaseState::FinalCycle2 )
        {
            stop();
        }
        else if(    state_ == PhaseState::Cycle2
                 && config_.failure_mode == FailureHandlingMode::StopOnFailure )
        {
            check_hausdorff_stopping_condition();
        }

        if ( !is_stopped() )
        {
            ed_.phase_step_count = 0;
            state_ = PhaseState::Cycle1;
        }
    }
}

void ActiveContour::check_hausdorff_stopping_condition()
{
    assert(    state_ == PhaseState::Cycle2
            && config_.failure_mode == FailureHandlingMode::StopOnFailure );

    const float step_delta = float(ed_.step_count - ed_.previous_step_count);

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
            ed_.l_out_shape.push_back( from_ContourPoint( point ) );
        }

        ed_.l_out_shape.calculate_centroid();

        calculate_shapes_intersection();

        HausdorffDistance hd( ed_.l_out_shape,
                              ed_.previous_shape,
                              ed_.intersection );

        const float size_factor = 100.f / diagonal;
        ed_.hausdorff_quantile = size_factor * hd.get_hausdorff_quantile(80);
        ed_.centroids_distance = size_factor * hd.get_centroids_distance();
        const float delta_quantile = ed_.previous_quantile - ed_.hausdorff_quantile;

        if ( ( ed_.centroids_distance < 1.f &&
               ed_.hausdorff_quantile < 1.f )
             ||
             ( ed_.centroids_distance < 1.f &&
               ed_.hausdorff_quantile < 2.f &&
               delta_quantile < 0.f ) )
        {
            ed_.stopping_status = StoppingStatus::Hausdorff;
            stop();
        }

        // swap the shapes in constant time O(1)
        // to prepare data for the next periodic check of the hausdorff condition
        ed_.l_out_shape.swap(ed_.previous_shape);
        ed_.previous_step_count = ed_.step_count;
        ed_.previous_quantile = ed_.hausdorff_quantile;
    }
}

void ActiveContour::calculate_shapes_intersection()
{
    assert( state_ == PhaseState::Cycle2 );

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

void ActiveContour::restart()
{
    // Intended to be used in RecoverOnFailure mode (video tracking).

    state_ = PhaseState::Cycle1;
    ed_.resetExecutionState();
}

}
