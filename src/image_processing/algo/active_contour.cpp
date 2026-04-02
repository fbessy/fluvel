// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "active_contour.hpp"
#include "ac_types.hpp"
#include "hausdorff_distance.hpp"
#include "neighborhood.hpp"
#include "speed_model.hpp"

#include <cassert>
#include <cstddef>
#include <cstdlib>

namespace fluvel_ip
{

// Definitions
ActiveContour::ActiveContour(ContourData initialContour, std::unique_ptr<ISpeedModel> speedModel,
                             const ActiveContourParams& configuration)
    : cd_(std::move(initialContour))
    , speedModel_(std::move(speedModel))
    , params_(configuration)
    , internalKernel_(
          makeInternalKernelOffsets(configuration.diskRadius, initialContour.phi().width()))
    , switchInMapping_(BoundarySwitchMapping::makeSwitchIn(cd_))
    , switchOutMapping_(BoundarySwitchMapping::makeSwitchOut(cd_))
    , currentMapping_(&switchInMapping_)
    , state_(PhaseState::Cycle1)
    , ed_(cd_)
{
    pendingBoundaryPoints_.reserve(cd_.l_out().capacity());

    if (params_.hasCycle2)
    {
        stepsPerCycle_ = params_.Na + params_.Ns;
    }
    else
    {
        stepsPerCycle_ = params_.Na;
    }
}

ActiveContour::~ActiveContour() = default;

void ActiveContour::handleFailure()
{
    if (!cd_.empty())
        return;

    ed_.stoppingStatus = StoppingStatus::EmptyListFailure;
    stop();

    // Recovery mode: the failure stops the current iteration,
    // but contour data are repaired to allow a future restart.
    if (params_.failureMode == FailureHandlingMode::RecoverOnFailure)
        cd_.define_from_ellipse();
}

void ActiveContour::enforceIterationLimit()
{
    assert(state_ == PhaseState::Cycle1);

    // Condition to handle and avoid infinite loop of the method converge(),
    // The convergence is data-dependent and therefore not guaranteed.
    if (ed_.stepCount >= ed_.maxStepCount)
    {
        ed_.stoppingStatus = StoppingStatus::MaxIteration;

        if (params_.hasCycle2)
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

    while (!isStopped())
    {
        while (state_ == PhaseState::Cycle1)
        {
            stepCycle1();
        }

        while (state_ == PhaseState::Cycle2 || state_ == PhaseState::FinalCycle2)
        {
            stepCycle2();
        }
    }
}

void ActiveContour::runSteps(int n_steps)
{
    if (n_steps < 1)
    {
        n_steps = 1;
    }

    for (int i = 0; i < n_steps; ++i)
    {
        if (isStopped())
            return;

        if (state_ == PhaseState::Cycle1)
        {
            stepCycle1();
        }
        else if (state_ == PhaseState::Cycle2 || state_ == PhaseState::FinalCycle2)
        {
            stepCycle2();
        }
    }
}

void ActiveContour::step()
{
    runSteps(1);
}

void ActiveContour::runCycles(int n_cycles)
{
    if (n_cycles < 1)
    {
        n_cycles = 1;
    }

    int n_steps = n_cycles * stepsPerCycle_;

    runSteps(n_steps);
}

void ActiveContour::stepCycle1()
{
    assert(state_ == PhaseState::Cycle1);

    handleFailure();
    if (isStopped())
        return;

    enforceIterationLimit();
    if (state_ != PhaseState::Cycle1)
        return;

    speedModel_->onStepCycle1();

    bool didOutwardMove = directionalSubstep(SwitchDirection::In);
    bool didInwardMove = directionalSubstep(SwitchDirection::Out);

    ++ed_.stepCount;
    ++ed_.phaseStepCount;

    ed_.didMove = (didOutwardMove || didInwardMove);

    checkStateStep1();
}

void ActiveContour::stepCycle2()
{
    assert(state_ == PhaseState::Cycle2 || state_ == PhaseState::FinalCycle2);

    handleFailure();
    if (isStopped())
        return;

    directionalSubstep(SwitchDirection::In);
    directionalSubstep(SwitchDirection::Out);

    ++ed_.stepCount;
    ++ed_.phaseStepCount;

    updateStateCycle2();
}

void ActiveContour::selectCurrentMapping(SwitchDirection direction)
{
    assert(!isStopped());

    if (direction == SwitchDirection::In)
        currentMapping_ = &switchInMapping_;
    else if (direction == SwitchDirection::Out)
        currentMapping_ = &switchOutMapping_;
}

bool ActiveContour::directionalSubstep(SwitchDirection direction)
{
    assert(!isStopped());

    bool didMove = false;

    selectCurrentMapping(direction);
    const auto& cm = currentMapping();

    auto& listToScan = cm.fromBoundary;
    auto& adjacentList = cm.toBoundary;

    pendingBoundaryPoints_.clear();

    updateSpeeds(listToScan);

    for (std::size_t i = 0; i < listToScan.size();)
    {
        auto& point = listToScan[i];

        if (point.speed() == cm.requiredSpeedSign)
        {
            didMove = true;

            speedModel_->onSwitch(point, direction);

            switchBoundaryPoint(point);
        }
        else
        {
            ++i;
        }
    }

    listToScan.insert(listToScan.end(), pendingBoundaryPoints_.begin(),
                      pendingBoundaryPoints_.end());

    if (didMove)
        cd_.eliminateRedundantPoints(adjacentList, cm.redundantToRegionVal);

    return didMove;
}

void ActiveContour::switchBoundaryPoint(ContourPoint& point)
{
    assert(!isStopped());

    const int x = point.x();
    const int y = point.y();

    const int w = cd_.phi().width();
    const int h = cd_.phi().height();

    const auto connected = cd_.connectivity();

    // ---------- FAST PATH ----------
    if (x > 0 && x + 1 < w && y > 0 && y + 1 < h)
    {
        // voisins 4-connectés
        for (const auto& d : kNeighbors4)
        {
            promoteRegionToBoundary(x + d.dx, y + d.dy);
        }

        // extension diagonale
        if (connected == Connectivity::Eight)
        {
            for (const auto& d : kNeighbors4Diag)
            {
                promoteRegionToBoundary(x + d.dx, y + d.dy);
            }
        }
    }
    else
    {
        // ---------- SLOW PATH (bords) ----------
        for (const auto& d : kNeighbors4)
        {
            const int nx = x + d.dx;
            const int ny = y + d.dy;

            if (nx < 0 || nx >= w || ny < 0 || ny >= h)
                continue;

            promoteRegionToBoundary(nx, ny);
        }

        if (connected == Connectivity::Eight)
        {
            for (const auto& d : kNeighbors4Diag)
            {
                const int nx = x + d.dx;
                const int ny = y + d.dy;

                if (nx < 0 || nx >= w || ny < 0 || ny >= h)
                    continue;

                promoteRegionToBoundary(nx, ny);
            }
        }
    }

    const auto& cm = currentMapping();

    // bascule du point courant
    cd_.phi().at(x, y) = cm.currentToAdjacentVal;

    // passage dans la boundary adjacente
    cm.toBoundary.emplace_back(x, y);

    point = cm.fromBoundary.back();
    cm.fromBoundary.pop_back();
}

void ActiveContour::promoteRegionToBoundary(int nx, int ny)
{
    assert(!isStopped());

    const auto& cm = currentMapping();

    auto& phiNeighbor = cd_.phi().at(nx, ny);

    // neighbor ∈ region ?
    if (phiNeighbor == cm.neighborFromRegionVal)
    {
        phiNeighbor = cm.neighbor_to_boundary_val;

        // neighbor pending to the scanned boundary
        pendingBoundaryPoints_.emplace_back(nx, ny);
    }
}

void ActiveContour::updateSpeeds(Contour& boundary)
{
    assert(!isStopped());

    if (state_ == PhaseState::Cycle1)
    {
        computeSpeeds(boundary);
    }
    else if (state_ == PhaseState::Cycle2 || state_ == PhaseState::FinalCycle2)
    {
        computeInternalSpeeds(boundary);
    }
}

void ActiveContour::computeSpeeds(Contour& boundary)
{
    assert(state_ == PhaseState::Cycle1);

    for (auto& point : boundary)
        speedModel_->computeSpeed(point, cd_.phi());
}

void ActiveContour::computeInternalSpeeds(Contour& boundary)
{
    assert(state_ == PhaseState::Cycle2 || state_ == PhaseState::FinalCycle2);

    for (auto& point : boundary)
    {
        computeInternalSpeed(point);
    }
}

InternalKernel ActiveContour::makeInternalKernelOffsets(int diskRadius, int grid_width)
{
    const int r = diskRadius;
    const int w = grid_width;

    InternalKernel kernel;
    kernel.offsets.reserve(static_cast<std::size_t>((2 * r + 1) * (2 * r + 1)));

    kernel.support.min_dx = r;
    kernel.support.max_dx = -r;
    kernel.support.min_dy = r;
    kernel.support.max_dy = -r;

    for (int dy = -r; dy <= r; ++dy)
    {
        for (int dx = -r; dx <= r; ++dx)
        {
            if (dx * dx + dy * dy <= r * r)
            {
                kernel.offsets.push_back(dy * w + dx);

                kernel.support.min_dx = std::min(kernel.support.min_dx, dx);
                kernel.support.max_dx = std::max(kernel.support.max_dx, dx);
                kernel.support.min_dy = std::min(kernel.support.min_dy, dy);
                kernel.support.max_dy = std::max(kernel.support.max_dy, dy);
            }
        }
    }

    return kernel;
}

void ActiveContour::computeInternalSpeed(ContourPoint& point)
{
    assert(state_ == PhaseState::Cycle2 || state_ == PhaseState::FinalCycle2);

    // Internal speed Fint is computed using a local majority vote
    // on the interior/exterior labeling of neighboring pixels.
    // This is equivalent to a smoothed Heaviside convolution
    // described in the reference paper.

    // Note: the direction is intentionally inverted.
    // If the neighborhood is mostly inside, the boundary locally
    // protrudes outward and must be pushed outward to smooth curvature.

    const int base = cd_.phi().offset(point.x(), point.y());

    int neighborOffset;

    int inside = 0;
    int outside = 0;

    if (internalKernel_.fully_inside(point.x(), point.y(), cd_.phi().width(), cd_.phi().height()))
    {
        // fast path without all neighbors existence checks

        for (int delta : internalKernel_.offsets)
        {
            neighborOffset = base + delta;

            PhiValue v = cd_.phi()[neighborOffset];

            if (phi_value::isInside(v))
                ++inside;
            else
                ++outside;
        }
    }
    else
    {
        for (int delta : internalKernel_.offsets)
        {
            neighborOffset = base + delta;

            if (cd_.phi().valid(neighborOffset) /* all checks */)
            {
                PhiValue v = cd_.phi()[neighborOffset];

                if (phi_value::isInside(v))
                    ++inside;
                else
                    ++outside;
            }
        }
    }

    // intentionally inverted, here.
    if (inside > outside)
        point.setSpeed(SpeedValue::GoOutward);
    else if (outside > inside)
        point.setSpeed(SpeedValue::GoInward);
    else
        point.setSpeed(SpeedValue::NoMove);
}

void ActiveContour::stop()
{
    assert(state_ != PhaseState::Stopped);

    state_ = PhaseState::Stopped;
}

void ActiveContour::checkStateStep1()
{
    assert(state_ == PhaseState::Cycle1);

    if (!ed_.didMove)
    {
        ed_.stoppingStatus = StoppingStatus::ListsConverged;

        if (params_.hasCycle2)
        {
            ed_.phaseStepCount = 0;
            state_ = PhaseState::FinalCycle2;
        }
        else
        {
            stop();
        }
    }
    else if (ed_.phaseStepCount >= params_.Na && params_.hasCycle2)
    {
        ed_.phaseStepCount = 0;
        state_ = PhaseState::Cycle2;
    }
}

void ActiveContour::updateStateCycle2()
{
    assert(state_ == PhaseState::Cycle2 || state_ == PhaseState::FinalCycle2);

    // if at the end of one cycle 2
    if (ed_.phaseStepCount >= params_.Ns)
    {
        if (state_ == PhaseState::FinalCycle2)
        {
            stop();
        }
        else if (state_ == PhaseState::Cycle2 &&
                 params_.failureMode == FailureHandlingMode::StopOnFailure)
        {
            check_hausdorff_stopping_condition();
        }

        if (!isStopped())
        {
            ed_.phaseStepCount = 0;
            state_ = PhaseState::Cycle1;
        }
    }
}

void ActiveContour::check_hausdorff_stopping_condition()
{
    assert(state_ == PhaseState::Cycle2 &&
           params_.failureMode == FailureHandlingMode::StopOnFailure);

    const float stepDelta = float(ed_.stepCount - ed_.previous_step_count);

    const int w = cd_.phi().width();
    const int h = cd_.phi().height();
    const int diagonal = Shape::get_grid_diagonal(w, h);

    // normalize in function of l_outshape/phi size
    // to handle different images sizes.
    if (stepDelta >= diagonal / 20.f)
    {
        ed_.l_out_shape.clear();

        for (const auto& point : cd_.l_out())
        {
            ed_.l_out_shape.push_back(from_ContourPoint(point));
        }

        ed_.l_out_shape.calculate_centroid();

        calculateShapesIntersection();

        HausdorffDistance hd(ed_.l_out_shape, ed_.previousShape, ed_.intersection);

        const float sizeFactor = 100.f / diagonal;
        ed_.hausdorffQuantile = sizeFactor * hd.hausdorffQuantile(80);
        ed_.relativeCentroidDistance = sizeFactor * hd.get_centroids_distance();
        const float delta_quantile = ed_.previousQuantile - ed_.hausdorffQuantile;

        if ((ed_.relativeCentroidDistance < 1.f && ed_.hausdorffQuantile < 1.f) ||
            (ed_.relativeCentroidDistance < 1.f && ed_.hausdorffQuantile < 2.f &&
             delta_quantile < 0.f))
        {
            ed_.stoppingStatus = StoppingStatus::Hausdorff;
            stop();
        }

        // swap the shapes in constant time O(1)
        // to prepare data for the next periodic check of the hausdorff condition
        ed_.l_out_shape.swap(ed_.previousShape);
        ed_.previous_step_count = ed_.stepCount;
        ed_.previousQuantile = ed_.hausdorffQuantile;
    }
}

void ActiveContour::calculateShapesIntersection()
{
    assert(state_ == PhaseState::Cycle2);

    ed_.intersection.clear();

    for (const auto& point : ed_.previousShape.get_points())
    {
        // if a point of ed.previousShape ∈ ed.l_out_shape
        if (cd_.phi().at(point.x, point.y) == PhiValue::ExteriorBoundary)
        {
            ed_.intersection.insert(point);
        }
    }
}

void ActiveContour::update(ImageView image)
{
    state_ = PhaseState::Cycle1;
    ed_.resetExecutionState();

    speedModel_->onImageChanged(image, cd_);
}

void ActiveContour::fillDiagnostics(ContourDiagnostics& d) const
{
    d.stepCount = ed_.stepCount;
    d.state = state_;
    d.stoppingStatus = ed_.stoppingStatus;

    d.hausdorffQuantile = ed_.hausdorffQuantile;
    d.relativeCentroidDistance = ed_.relativeCentroidDistance;

    d.contourSize = cd_.l_out().size() + cd_.l_in().size();

    speedModel_->fillDiagnostics(d);
}

} // namespace fluvel_ip
