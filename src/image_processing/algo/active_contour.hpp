// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ac_types.hpp"
#include "contour_data.hpp"
#include "contour_diagnostics.hpp"
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
    ActiveContour(const ContourData& initialState, const AcConfig& config);

    //! Constructor to initialize the active contour from an initial contour (#phi, #l_in and
    //! #l_out) with a move semantic.
    ActiveContour(ContourData&& initialState, const AcConfig& config);

    //! Destructor.
    virtual ~ActiveContour() = default;

    //! Handles a failure case.
    void handleFailure();

    //! Runs or evolves the active contour until it reaches a terminal state.
    void converge();

    //! The active contour evolves to one iteration.
    void step();

    //! Runs the active contour for a fixed number of full cycles (cycle1 + cycle2).
    //! Ensures the contour is geometrically stable at the end of each cycle2 (used for video
    //! tracking).
    void runCycles(int n_cycles);

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

    virtual void fillDiagnostics(ContourDiagnostics& d) const;

    //! Gets if the active contour reaches the final state.
    bool isStopped() const
    {
        return state_ == PhaseState::Stopped;
    }

    bool isFirstIteration() const
    {
        return ed_.stepCount == 0;
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
    void runSteps(int n_steps);

    //! Performs one elementary step in Cycle1 (external / data-dependent evolution, speed Fd).
    void stepCycle1();

    //! Performs one elementary step in Cycle1 (external / data-dependent evolution, speed Fd).
    void stepCycle2();

    //! It selects the context.
    void selectContext(BoundarySwitch ctxChoice);

    //! Get the selected context.
    const BoundarySwitchContext& context()
    {
        return *ctx_;
    }

    //! Performs one directional topological update step (in or out).
    //! The step includes velocity computation, boundary switching,
    //! and adjacent boundary cleanup.
    bool directionalSubstep(BoundarySwitch ctxChoice);

    //! Computes the speed for all points of a boundary list #l_out or #l_in.
    void computeSpeed(Contour& boundary);

    //! Computes the external speed Fd for all points of a boundary list #l_out or #l_in.
    void computeExternalSpeedFd(Contour& boundary);

    //! Computes the external speed \a Fd for a current point (\a x,\a y) of #l_out or #l_in.
    virtual void computeExternalSpeedFd(ContourPoint& point);

    //! Computes the internal speed  Fint for all points of a boundary list #l_out or #l_in.
    void computeInternalSpeedFint(Contour& boundary);

    //! Computes the internal speed  Fint for a current point (\a x,\a y) of #l_out or #l_in.
    void computeInternalSpeedFint(ContourPoint& point);

    //! Generic method to handle outward / inward local movement of a current boundary point (of
    //! #l_out or #l_in) and to switch it from one boundary list to the other.
    void switchBoundaryPoint(ContourPoint& point);

    //! Promote a neighboring region point to boundary.
    void promoteRegionToBoundary(int nx, int ny);

    //! Specific step for each iteration in cycle 1.
    virtual void do_specific_cycle1()
    {
    }

    //! Specific step when switch in or a switch out procedure is performed.
    virtual void doSpecificWhenSwitch(const ContourPoint& /*point*/, BoundarySwitch)
    {
    }

    //! Stops the active contour and puts it in a terminal state.
    //! After this call, step(), converge(), and runCycles() have no effect.
    void stop();

    //! Check if iteration limit is not reached.
    void enforceIterationLimit();

    //! Calculates the active contour state at the end of each iteration of a cycle 1.
    void checkStateStep1();

    //! Updates the active contour state at the end of a cycle 2.
    void updateStateCycle2();

    //! Check an internal condition, based on the contour state rather than image data,
    //! to stop the algorithm if the nominal convergence condition is not reached.
    void check_hausdorff_stopping_condition();

    //! Computes the shapes intersection between #ed.l_out_shape and #ed.previousShape
    //! to speed up the hausdorff distance computation.
    void calculateShapesIntersection();

    //! Build kernel offsets.
    static InternalKernel makeInternalKernelOffsets(int diskRadius, int grid_width);

    //! To transformate active contour data point to the points for the Hausdorff distance.
    static Point2D_i from_ContourPoint(const ContourPoint& point);

    //! Generic configuration of the active contour.
    const AcConfig config_;

    //! Precomputed disk-shaped kernel offsets for internal smoothing (Fint).
    const InternalKernel internalKernel_;

    //! BoundarySwitchContext for the procedure switch_in.
    const BoundarySwitchContext ctxIn_;

    //! BoundarySwitchContext for the procedure switch_out.
    const BoundarySwitchContext ctxOut_;

    //! A selector (pointer) to perform a switch generically. It pointes to ctxIn_ or ctxOut_.
    const BoundarySwitchContext* ctx_;

    //! Temporary points to add after each scan of the list #l_in or #l_out.
    Contour activeBoundaryStaging_;

    //! Number of iterations in one cycle1-cycle2.
    int steps_per_cycle_;

    //! Evolution state of the active contour given at a current iteration.
    //!  There are 4 states : Cycle1, Cycle2, FinalCycle2 and Stopped.
    PhaseState state_;

    //! Evolution data of the active contour used by #checkStateStep1() and
    //! #updateStateCycle2() to calculate #state.
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
