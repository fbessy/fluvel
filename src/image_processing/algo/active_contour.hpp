// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour_types.hpp"
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

/**
 * @brief Initial allocation size for speed arrays.
 *
 * Used to avoid frequent reallocations when computing speeds.
 */
constexpr size_t kInitialSpeedArrayAllocSize = 10000u;

/**
 * @brief Mapping used to perform generic boundary switching operations.
 *
 * This structure encapsulates all required parameters to perform
 * either a "switch in" or "switch out" operation on contour boundaries.
 */
struct BoundarySwitchMapping
{
    Contour& fromBoundary;          //!< Source boundary (points to remove from).
    Contour& toBoundary;            //!< Destination boundary (points to add to).
    SpeedValue requiredSpeedSign;   //!< Required speed sign to trigger the switch.
    PhiValue currentToAdjacentVal;  //!< Phi value for adjacent transition.
    PhiValue neighborFromRegionVal; //!< Phi value for neighbor in source region.
    PhiValue neighborToBoundaryVal; //!< Phi value for neighbor in destination boundary.
    PhiValue redundantToRegionVal;  //!< Phi value for redundant region update.

    /**
     * @brief Creates a mapping for a "switch in" operation.
     *
     * @param cd Contour data.
     * @return Mapping configured for inward switching.
     */
    static BoundarySwitchMapping makeSwitchIn(ContourData& cd)
    {
        return {cd.outerBoundary(),         cd.innerBoundary(),      SpeedValue::GoOutward,
                PhiValue::InteriorBoundary, PhiValue::OutsideRegion, PhiValue::ExteriorBoundary,
                PhiValue::InsideRegion};
    }

    /**
     * @brief Creates a mapping for a "switch out" operation.
     *
     * @param cd Contour data.
     * @return Mapping configured for outward switching.
     */
    static BoundarySwitchMapping makeSwitchOut(ContourData& cd)
    {
        return {cd.innerBoundary(),         cd.outerBoundary(),     SpeedValue::GoInward,
                PhiValue::ExteriorBoundary, PhiValue::InsideRegion, PhiValue::InteriorBoundary,
                PhiValue::OutsideRegion};
    }
};

/**
 * @brief Stores runtime evolution data of the active contour.
 *
 * This structure tracks iteration counts, geometric evolution,
 * convergence metrics, and stopping conditions.
 */
struct EvolutionData
{
    int phaseStepCount{0};  //!< Number of iterations within the current phase.
    int stepCount{0};       //!< Total number of iterations since initialization.
    const int maxStepCount; //!< Maximum allowed number of iterations.

    bool didMove{true}; //!< Indicates if the contour moved during Cycle1.

    Shape l_out_shape;   //!< Outer boundary shape at the end of the current Cycle2.
    Shape previousShape; //!< Outer boundary shape at the end of the previous Cycle2.

    int previous_step_count{0}; //!< Step count at the previous Cycle2.

    float previousQuantile{
        std::numeric_limits<float>::quiet_NaN()}; //!< Previous Hausdorff quantile.
    float hausdorffQuantile{
        std::numeric_limits<float>::quiet_NaN()}; //!< Current Hausdorff quantile.
    float relativeCentroidDistance{
        std::numeric_limits<float>::quiet_NaN()}; //!< Normalized centroid distance.

    PointSet intersection; //!< Intersection between current and previous shapes.

    StoppingStatus stoppingStatus{StoppingStatus::None}; //!< Current stopping condition.

    /**
     * @brief Constructor.
     *
     * Initializes buffers and computes the maximum number of iterations
     * based on image dimensions.
     *
     * @param cd Contour data.
     */
    EvolutionData(const ContourData& cd)
        : maxStepCount(5 * std::max(cd.phi().width(), cd.phi().height()))
    {
        l_out_shape.reserve(cd.outerBoundary().capacity());
        previousShape.reserve(cd.outerBoundary().capacity());
        intersection.reserve(cd.outerBoundary().capacity());
    }

    /**
     * @brief Resets runtime execution state.
     *
     * Keeps allocated buffers but resets iteration counters and status.
     * Typically used for video tracking scenarios.
     */
    void resetExecutionState()
    {
        phaseStepCount = 0;
        stepCount = 0;
        stoppingStatus = StoppingStatus::None;
    }
};

/**
 * @brief Active contour (level-set based) implementation.
 *
 * This class implements a discrete active contour algorithm using
 * two alternating phases:
 * - Cycle1: data-driven evolution
 * - Cycle2: smoothing / regularization
 *
 * It supports both image segmentation and video tracking scenarios.
 */
class ActiveContour
{
public:
    /**
     * @brief Constructs an active contour instance.
     *
     * @param initialContour Initial contour data (copied).
     * @param speedModel External speed model.
     * @param params Algorithm parameters.
     */
    ActiveContour(ContourData initialContour, std::unique_ptr<ISpeedModel> speedModel,
                  const ActiveContourParams& params);

    /**
     * @brief Destructor.
     */
    virtual ~ActiveContour();

    /**
     * @brief Updates the contour with a new input image.
     *
     * The image must match the dimensions of the level-set function.
     *
     * @param image Input image.
     */
    void update(const ImageView& image);

    /**
     * @brief Checks for contour-related failure conditions.
     */
    void checkContourFailure();

    /**
     * @brief Checks for failures in the external speed model.
     */
    void checkSpeedModelFailure();

    /**
     * @brief Runs the contour evolution until a stopping condition is reached.
     */
    void converge();

    /**
     * @brief Performs a single iteration step.
     */
    void step();

    /**
     * @brief Runs a fixed number of full cycles (Cycle1 + Cycle2).
     *
     * Used for stable updates in video tracking.
     *
     * @param n_cycles Number of cycles.
     */
    void runCycles(int n_cycles);

    /**
     * @brief Exports the outer boundary as a geometric contour.
     *
     * @return Copy of the outer boundary.
     */
    ExportedContour exportOuterBoundary() const
    {
        return cd_.exportOuterBoundary();
    }

    /**
     * @brief Exports the inner boundary as a geometric contour.
     *
     * @return Copy of the inner boundary.
     */
    ExportedContour exportInnerBoundary() const
    {
        return cd_.exportInnerBoundary();
    }

    /**
     * @brief Returns the discrete level-set function.
     */
    const DiscreteLevelSet& phi() const
    {
        return cd_.phi();
    }

    /**
     * @brief Returns the outer boundary.
     */
    const Contour& outerBoundary() const
    {
        return cd_.outerBoundary();
    }

    /**
     * @brief Returns the inner boundary.
     */
    const Contour& innerBoundary() const
    {
        return cd_.innerBoundary();
    }

    /**
     * @brief Fills diagnostic information.
     *
     * @param d Output diagnostics structure.
     */
    void fillDiagnostics(ContourDiagnostics& d) const;

    /**
     * @brief Indicates whether the contour has reached a terminal state.
     */
    bool isStopped() const
    {
        return state_ == PhaseState::Stopped;
    }

    /**
     * @brief Indicates whether this is the first iteration.
     */
    bool isFirstIteration() const
    {
        return ed_.stepCount == 0;
    }

private:
    /**
     * @brief Runs a fixed number of elementary steps.
     */
    void runSteps(int n_steps);

    /**
     * @brief Performs one Cycle1 step (data-driven evolution).
     */
    void stepCycle1();

    /**
     * @brief Performs one Cycle2 step (smoothing phase).
     */
    void stepCycle2();

    /**
     * @brief Selects the current boundary mapping (in/out).
     */
    void selectCurrentMapping(SwitchDirection direction);

    /**
     * @brief Returns the currently selected mapping.
     */
    const BoundarySwitchMapping& currentMapping() const
    {
        return *currentMapping_;
    }

    /**
     * @brief Performs one directional substep (inward or outward).
     *
     * @return True if any movement occurred.
     */
    bool directionalSubstep(SwitchDirection direction);

    /**
     * @brief Updates speeds for all points in a boundary.
     */
    void updateSpeeds(Contour& boundary);

    /**
     * @brief Generic speed computation loop.
     */
    template <typename SpeedModel>
    void computeSpeeds(Contour& boundary, SpeedModel&& model)
    {
        for (auto& point : boundary)
            model.computeSpeed(point, cd_.phi());
    }

    /**
     * @brief Switches a boundary point between inner and outer lists.
     */
    void switchBoundaryPoint(ContourPoint& point);

    /**
     * @brief Promotes a region point to a boundary point.
     */
    void promoteRegionToBoundary(int nx, int ny);

    /**
     * @brief Stops the contour evolution.
     */
    void stop();

    /**
     * @brief Enforces iteration limit.
     */
    void enforceIterationLimit();

    /**
     * @brief Updates state after Cycle1.
     */
    void checkStateStep1();

    /**
     * @brief Updates state after Cycle2.
     */
    void updateStateCycle2();

    /**
     * @brief Checks Hausdorff-based stopping condition.
     */
    void checkHausdorffStoppingCondition();

    /**
     * @brief Computes intersection between current and previous shapes.
     */
    void calculateShapesIntersection();

    ContourData cd_; //!< Core contour representation.

    std::unique_ptr<ISpeedModel> externalSpeedModel_; //!< External speed model.

    const ActiveContourParams params_; //!< Algorithm parameters.

    MajorityInternalSpeed internalSpeedModel_; //!< Internal smoothing speed model.

    const BoundarySwitchMapping switchInMapping_;  //!< Mapping for inward switch.
    const BoundarySwitchMapping switchOutMapping_; //!< Mapping for outward switch.

    const BoundarySwitchMapping* currentMapping_; //!< Currently selected mapping.

    Contour pendingBoundaryPoints_; //!< Temporary storage for boundary updates.

    int stepsPerCycle_; //!< Number of steps per full cycle.

    PhaseState state_; //!< Current phase state.

    EvolutionData ed_; //!< Evolution tracking data.
};

} // namespace fluvel_ip
