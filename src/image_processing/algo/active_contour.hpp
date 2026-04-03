// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ac_types.hpp"
#include "contour_data.hpp"
#include "contour_diagnostics.hpp"
#include "majority_internal_speed.hpp"
#include "point_containers.hpp"
#include "shape.hpp"

#include <cstddef>
#include <memory>

namespace fluvel_ip
{

class ISpeedModel;

constexpr size_t kInitialSpeedArrayAllocSize = 10000u;

//! \struct BoundarySwitchMapping to perform generically a switch in or a switch out.
struct BoundarySwitchMapping
{
    Contour& fromBoundary;
    Contour& toBoundary;
    SpeedValue requiredSpeedSign;
    PhiValue currentToAdjacentVal;
    PhiValue neighborFromRegionVal;
    PhiValue neighborToBoundaryVal;
    PhiValue redundantToRegionVal;

    static BoundarySwitchMapping makeSwitchIn(ContourData& cd)
    {
        return {cd.outerBoundary(),         cd.innerBoundary(),      SpeedValue::GoOutward,
                PhiValue::InteriorBoundary, PhiValue::OutsideRegion, PhiValue::ExteriorBoundary,
                PhiValue::InsideRegion};
    }

    static BoundarySwitchMapping makeSwitchOut(ContourData& cd)
    {
        return {cd.innerBoundary(),         cd.outerBoundary(),     SpeedValue::GoInward,
                PhiValue::ExteriorBoundary, PhiValue::InsideRegion, PhiValue::InteriorBoundary,
                PhiValue::OutsideRegion};
    }
};

//! \class EvolutionData
//! Holds the evolution data of the active contour.
struct EvolutionData
{
    //! Iterations number in a cycle (cycle 1 or cycle 2). It is set to 0 at the end of one
    //! cycle.
    int phaseStepCount{0};

    //! Total number of iterations the active contour has evolved from the initial contour.
    int stepCount{0};

    //! Maximum number of times the active contour can evolve.
    const int maxStepCount;

    //! Boolean egals to true if the active contour evolves in one way (at least) in cycle 1.
    bool didMove{true};

    //! outerBoundary shape at the end of the cycle 2.
    Shape l_out_shape;

    //! outerBoundary shape at the end of the previous cycle 2.
    Shape previousShape;

    //! Total number of iterations the active contour has evolved from the initial contour
    //! at the end of the previous cycle 2.
    int previous_step_count{0};

    //! Hausdorff quantile
    //! at the end of the previous cycle 2.
    float previousQuantile{std::numeric_limits<float>::quiet_NaN()};

    //! Hausdorff quantile
    //! at the end of the cycle 2. It is a normalized value, divided by the diagonal size of
    //! #phi, in percent.
    float hausdorffQuantile{std::numeric_limits<float>::quiet_NaN()};

    //! Centroids gap between #l_out_shape and previousShape#. It is a normalized value,
    //! divided by the diagonal size of #phi, in percent.
    float relativeCentroidDistance{std::numeric_limits<float>::quiet_NaN()};

    //! Intersection, i.e common points between #l_out_shape and #previousShape.
    PointSet intersection;

    //! Stopping condition status.
    StoppingStatus stoppingStatus{StoppingStatus::None};

    //! Constructor.
    EvolutionData(const ContourData& cd)
        : maxStepCount(5 * std::max(cd.phi().width(), cd.phi().height()))
    {
        l_out_shape.reserve(cd.outerBoundary().capacity());
        previousShape.reserve(cd.outerBoundary().capacity());
        intersection.reserve(cd.outerBoundary().capacity());
    }

    //! Reset execution state of evolution data. Used multiple times for video tracking.
    void resetExecutionState()
    {
        phaseStepCount = 0;
        stepCount = 0;
        stoppingStatus = StoppingStatus::None;
    }
};

class ActiveContour
{
public:
    //! Constructor to initialize the active contour from an initial contour (#phi, #innerBoundary
    //! and #outerBoundary) with a copy semantic.
    ActiveContour(ContourData initialContour, std::unique_ptr<ISpeedModel> speedModel,
                  const ActiveContourParams& params);

    //! Destructor.
    virtual ~ActiveContour();

    //! Updates the model with a new image matching phi's dimensions.
    void update(ImageView image);

    //! Checks a generic failure case.
    void checkContourFailure();

    //! Checks external speed model failures.
    void checkSpeedModelFailure();

    //! Runs or evolves the active contour until it reaches a terminal state.
    void converge();

    //! The active contour evolves to one iteration.
    void step();

    //! Runs the active contour for a fixed number of full cycles (cycle1 + cycle2).
    //! Ensures the contour is geometrically stable at the end of each cycle2 (used for video
    //! tracking).
    void runCycles(int n_cycles);

    //! Export the boundary list outerBoundary_ as a copied geometric representation.
    ExportedContour export_l_out() const
    {
        return cd_.export_l_out();
    }

    //! Export the boundary list innerBoundary_ as a copied geometric representation.
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
    const Contour& outerBoundary() const
    {
        return cd_.outerBoundary();
    }
    //! Getter for the list of offset points representing the interior boundary.
    const Contour& innerBoundary() const
    {
        return cd_.innerBoundary();
    }

    void fillDiagnostics(ContourDiagnostics& d) const;

    //! Gets if the active contour reaches the final state.
    bool isStopped() const
    {
        return state_ == PhaseState::Stopped;
    }

    bool isFirstIteration() const
    {
        return ed_.stepCount == 0;
    }

private:
    //! Runs the active contour for a fixed number of elementary steps.
    //! Intended for incremental updates (e.g. video tracking).
    void runSteps(int n_steps);

    //! Performs one elementary step in Cycle1 (external / data-dependent evolution, speed Fd).
    void stepCycle1();

    //! Performs one elementary step in Cycle1 (external / data-dependent evolution, speed Fd).
    void stepCycle2();

    //! It selects the current mapping.
    void selectCurrentMapping(SwitchDirection direction);

    //! Get the selected current mapping.
    const BoundarySwitchMapping& currentMapping() const
    {
        return *currentMapping_;
    }

    //! Performs one directional topological update step (in or out).
    //! The step includes velocity computation, boundary switching,
    //! and adjacent boundary cleanup.
    bool directionalSubstep(SwitchDirection direction);

    //! Computes the speed for all points of a boundary list #outerBoundary or #innerBoundary.
    void updateSpeeds(Contour& boundary);

    template <typename SpeedModel> void computeSpeeds(Contour& boundary, SpeedModel&& model)
    {
        for (auto& point : boundary)
            model.computeSpeed(point, cd_.phi());
    }

    //! Generic method to handle outward / inward local movement of a current boundary point (of
    //! #outerBoundary or #innerBoundary) and to switch it from one boundary list to the other.
    void switchBoundaryPoint(ContourPoint& point);

    //! Promote a neighboring region point to boundary.
    void promoteRegionToBoundary(int nx, int ny);

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

    //! Representation data of the active contour
    //! (discret level-set function phi, Lin and Lout)
    ContourData cd_;

    std::unique_ptr<ISpeedModel> externalSpeedModel_;

    //! Generic configuration of the active contour.
    const ActiveContourParams params_;

    MajorityInternalSpeed internalSpeedModel_;

    //! BoundarySwitchMapping for the procedure switch_in.
    const BoundarySwitchMapping switchInMapping_;

    //! BoundarySwitchMapping for the procedure switch_out.
    const BoundarySwitchMapping switchOutMapping_;

    //! A selector (pointer) to perform a switch generically. It pointes to switchInMapping_ or
    //! switchOutMapping_.
    const BoundarySwitchMapping* currentMapping_;

    //! Temporary points to add after each scan of the list #innerBoundary or #outerBoundary.
    Contour pendingBoundaryPoints_;

    //! Number of iterations in one cycle1-cycle2.
    int stepsPerCycle_;

    //! Evolution state of the active contour given at a current iteration.
    //!  There are 4 states : Cycle1, Cycle2, FinalCycle2 and Stopped.
    PhaseState state_;

    //! Evolution data of the active contour used by #checkStateStep1() and
    //! #updateStateCycle2() to calculate #state.
    EvolutionData ed_;
};

} // namespace fluvel_ip
