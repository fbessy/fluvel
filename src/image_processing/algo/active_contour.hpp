// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ac_types.hpp"
#include "contour_data.hpp"
#include "point.hpp"

#include <cstddef>

namespace ofeli_ip
{

constexpr size_t kInitialSpeedArrayAllocSize = 10000u;

class ActiveContour
{
public:
    //! Constructor to initialize the active contour from an initial contour (#phi, #l_in and
    //! #l_out) with a copy semantic.
    ActiveContour(const ContourData& initial_state, const AcConfig& config);

    //! Constructor to initialize the active contour from an initial contour (#phi, #l_in and
    //! #l_out) with a move semantic.
    ActiveContour(ContourData&& initial_state, const AcConfig& config);

    //! Destructor.
    virtual ~ActiveContour()
    {
    }

    //! Handles a failure case.
    void handle_failure();

    //! Runs or evolves the active contour until it reaches a terminal state.
    void converge();

    //! The active contour evolves to one iteration.
    void step();

    //! Runs the active contour for a fixed number of full cycles (cycle1 + cycle2).
    //! Ensures the contour is geometrically stable at the end of each cycle2 (used for video
    //! tracking).
    void run_cycles(int n_cycles);

    //! Export the boundary list l_out_ as a copied geometric representation.
    ExportedContour export_l_out() const
    {
        return cd_.export_l_out();
    }

    //! Export the boundary list l_in_ as a copied geometric representation.
    ExportedContour export_l_in() const
    {
        return cd_.export_l_in();
    }

    //! Getter for the discrete level-set function with only 4 PhiValue possible.
    const DiscreteLevelSet& phi() const
    {
        return cd_.phi();
    }

    //! Getter for the list of offset points representing the exterior boundary.
    const Contour& l_out() const
    {
        return cd_.l_out();
    }
    //! Getter for the list of offset points representing the interior boundary.
    const Contour& l_in() const
    {
        return cd_.l_in();
    }

    //! Getter method for #state.
    PhaseState state() const
    {
        return state_;
    }

    //! Gets if the active contour reaches the final state.
    bool is_stopped() const
    {
        return state_ == PhaseState::Stopped;
    }

    //! Getter method for #stopping_status.
    StoppingStatus stop_reason() const
    {
        return ed_.stopping_status;
    }

    //! Getter method for #step_count.
    int step_count() const
    {
        return ed_.step_count;
    }

    //! Getter method for #hausdorff_quantile.
    float hausdorff_quantile() const
    {
        return ed_.hausdorff_quantile;
    }

    //! Getter method for #centroids_distance.
    float centroids_distance() const
    {
        return ed_.centroids_distance;
    }

protected:
    //! restart the active contour, used for video tracking.
    void restart();

    //! Representation data of the active contour
    //! (discret level-set function phi, Lin and Lout)
    ContourData cd_;

private:
    //! Runs the active contour for a fixed number of elementary steps.
    //! Intended for incremental updates (e.g. video tracking).
    void run_steps(int n_steps);

    //! Performs one elementary step in Cycle1 (external / data-dependent evolution, speed Fd).
    void step_cycle1();

    //! Performs one elementary step in Cycle1 (external / data-dependent evolution, speed Fd).
    void step_cycle2();

    //! It selects the context.
    void select_context(BoundarySwitch ctx_choice);

    //! Get the selected context.
    const BoundarySwitchContext& context()
    {
        return *ctx_;
    }

    //! Performs one directional topological update step (in or out).
    //! The step includes velocity computation, boundary switching,
    //! and adjacent boundary cleanup.
    bool directional_substep(BoundarySwitch ctx_choice);

    //! Computes the speed for all points of a boundary list #l_out or #l_in.
    void compute_speed(Contour& boundary);

    //! Computes the external speed Fd for all points of a boundary list #l_out or #l_in.
    void compute_external_speed_Fd(Contour& boundary);

    //! Computes the external speed \a Fd for a current point (\a x,\a y) of #l_out or #l_in.
    virtual void compute_external_speed_Fd(ContourPoint& point);

    //! Computes the internal speed  Fint for all points of a boundary list #l_out or #l_in.
    void compute_internal_speed_Fint(Contour& boundary);

    //! Computes the internal speed  Fint for a current point (\a x,\a y) of #l_out or #l_in.
    void compute_internal_speed_Fint(ContourPoint& point);

    //! Generic method to handle outward / inward local movement of a current boundary point (of
    //! #l_out or #l_in) and to switch it from one boundary list to the other.
    void switch_boundary_point(ContourPoint& point);

    //! Promote a neighboring region point to boundary.
    void promote_region_to_boundary(int nx, int ny);

    //! Specific step for each iteration in cycle 1.
    virtual void do_specific_cycle1()
    {
    }

    //! Specific step when switch in or a switch out procedure is performed.
    virtual void do_specific_when_switch(const ContourPoint& /*point*/, BoundarySwitch)
    {
    }

    //! Stops the active contour and puts it in a terminal state.
    //! After this call, step(), converge(), and run_cycles() have no effect.
    void stop();

    //! Check if iteration limit is not reached.
    void enforce_iteration_limit();

    //! Calculates the active contour state at the end of each iteration of a cycle 1.
    void check_state_step1();

    //! Updates the active contour state at the end of a cycle 2.
    void update_state_cycle2();

    //! Check an internal condition, based on the contour state rather than image data,
    //! to stop the algorithm if the nominal convergence condition is not reached.
    void check_hausdorff_stopping_condition();

    //! Computes the shapes intersection between #ed.l_out_shape and #ed.previous_shape
    //! to speed up the hausdorff distance computation.
    void calculate_shapes_intersection();

    //! Build kernel offsets.
    static InternalKernel make_internal_kernel_offsets(int disk_radius, int grid_width);

    //! To transformate active contour data point to the points for the Hausdorff distance.
    static Point2D_i from_ContourPoint(const ContourPoint& point);

    //! Generic configuration of the active contour.
    const AcConfig config_;

    //! Precomputed disk-shaped kernel offsets for internal smoothing (Fint).
    const InternalKernel internal_kernel_;

    //! BoundarySwitchContext for the procedure switch_in.
    const BoundarySwitchContext ctx_in_;

    //! BoundarySwitchContext for the procedure switch_out.
    const BoundarySwitchContext ctx_out_;

    //! A selector (pointer) to perform a switch generically. It pointes to ctx_in_ or ctx_out_.
    const BoundarySwitchContext* ctx_;

    //! Temporary points to add after each scan of the list #l_in or #l_out.
    Contour active_boundary_staging_;

    //! Number of iterations in one cycle1-cycle2.
    int steps_per_cycle_;

    //! Evolution state of the active contour given at a current iteration.
    //!  There are 4 states : Cycle1, Cycle2, FinalCycle2 and Stopped.
    PhaseState state_;

    //! Evolution data of the active contour used by #check_state_step1() and
    //! #update_state_cycle2() to calculate #state.
    EvolutionData ed_;
};

inline Point2D_i ActiveContour::from_ContourPoint(const ContourPoint& point)
{
    return {point.x(), point.y()};
}

namespace speed_value
{

//! Gets a discrete speed.
constexpr SpeedValue get_discrete_speed(int speed);

constexpr SpeedValue get_discrete_speed(int speed)
{
    if (speed < 0)
        return SpeedValue::GoInward;
    if (speed > 0)
        return SpeedValue::GoOutward;
    return SpeedValue::NoMove;
}

} // namespace speed_value

} // namespace ofeli_ip
