// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "active_contour_worker.hpp"

#include "region_gray_speed_model.hpp"
#include "region_color_speed_model.hpp"
#include "speed_model.hpp"

#include "image_adapters.hpp"
#include "image_pipeline.hpp"
#include "image_view.hpp"

#include "elapsed_timer.hpp"

#include <QTimer>

#ifdef FLUVEL_DEBUG
#include <QDebug>
#endif

#include <cassert>

namespace fluvel_app
{

ActiveContourWorker::ActiveContourWorker()
    : QObject(nullptr)
    , workerTimer_(new QTimer(this))
{
    workerTimer_->setInterval(kWorkerPeriodMs);
    connect(workerTimer_, &QTimer::timeout, this, &ActiveContourWorker::onTimeout);
}

void ActiveContourWorker::restart()
{
    if (state_ == WorkerState::Initializing)
        return;

    setMode(RunMode::Interactive);

    const auto& fc = config_.processing;

    const bool hasRandom = fc.has_gaussian_noise || fc.has_salt_noise || fc.has_speckle_noise;

    if (processingDirty_ || hasRandom)
        applyProcessing();

    initializeActiveContour();

    if (state_ != WorkerState::Ready)
        return;

    updateDiagnostics();
    emitContour();

    initialShown_ = true;

    start();
}

void ActiveContourWorker::start()
{
    if (state_ == WorkerState::Uninitialized || state_ == WorkerState::Initializing)
        return;

    setState(WorkerState::Running);
    workerTimer_->start();
}

void ActiveContourWorker::togglePause()
{
    if (state_ == WorkerState::Uninitialized || state_ == WorkerState::Initializing)
        return;

    setMode(RunMode::Interactive);

    if (state_ == WorkerState::Running)
    {
        suspend();
    }
    else if (state_ == WorkerState::Suspended)
    {
        resume();
    }
    else if (state_ == WorkerState::Ready)
    {
        if (!initialShown_)
        {
            updateDiagnostics();
            emitContour();

            initialShown_ = true;
        }
        else
        {
            resume();
        }
    }
}

void ActiveContourWorker::step()
{
    if (state_ == WorkerState::Uninitialized || state_ == WorkerState::Initializing)
        return;

    setMode(RunMode::Interactive);

    if (state_ == WorkerState::Running)
    {
        suspend();
    }
    else if (state_ == WorkerState::Suspended)
    {
        performStep();
    }
    else if (state_ == WorkerState::Ready)
    {
        if (!initialShown_)
        {
            updateDiagnostics();
            emitContour();

            initialShown_ = true;
        }
        else
        {
            performStep();
        }
    }
}

void ActiveContourWorker::performStep()
{
    if (stepOnceAlgo())
    {
        finalizeAndPrepareNextRun();
    }
    else
    {
        updateDiagnostics();
        emitContour();
    }
}

void ActiveContourWorker::converge()
{
    if (state_ == WorkerState::Uninitialized || state_ == WorkerState::Initializing)
        return;

    if (state_ == WorkerState::Ready)
    {
        updateDiagnostics();
        emitContour();

        initialShown_ = true;
    }

    setMode(RunMode::Converge);

    if (state_ != WorkerState::Running)
        start();
}

bool ActiveContourWorker::stepOnceAlgo()
{
    assert(activeContour_ != nullptr);

    if (activeContour_->isStopped())
    {
        initializeActiveContour();
    }
    else
    {
        if (activeContour_->isFirstIteration())
        {
            isMeasuring_ = true;
            measurementTimer_.start();
        }

        activeContour_->step();
    }

    return activeContour_->isStopped();
}

void ActiveContourWorker::applyProcessing()
{
    workerTimer_->stop();
    setState(WorkerState::Initializing);

    processedImage_ = image_;

    if (image_.isNull())
    {
        processingDirty_ = false;
        return;
    }

#ifdef FLUVEL_DEBUG
    qDebug() << __FILE__ << " applyProcessing() " << __LINE__ << __func__;
#endif

    fluvel_ip::ImagePipeline preprocess;
    auto img = imageViewFromQImage(image_);

    preprocess.reset(img);
    preprocess.apply(img, config_.processing);

    processedImage_ = toQImageCopy(preprocess.outputView());

    //--------------------------------------------------
    // 5. Emit
    //--------------------------------------------------
    if (!processedImage_.isNull())
        emit processedImageReady(processedImage_);

    processingDirty_ = false;
}

void ActiveContourWorker::initialize(const QImage& image, const ImageComputeConfig& config)
{
    workerTimer_->stop();
    setState(WorkerState::Initializing);

    image_ = image;
    config_ = config;
    processingDirty_ = true;

    applyProcessing();

    initializeActiveContour();

    if (state_ != WorkerState::Ready)
        return;

    updateDiagnostics();
    emitContour();

    initialShown_ = true;
}

void ActiveContourWorker::initializeActiveContour()
{
    if (processedImage_.isNull())
        return;

    workerTimer_->stop();
    setState(WorkerState::Initializing);

    activeContour_.reset();
    resetMeasurement();

    assert(processedImage_.size() == config_.initialPhi.size());

    if (processedImage_.size() != config_.initialPhi.size())
    {
        config_.initialPhi = config_.initialPhi.scaled(
            processedImage_.size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }

    assert(!config_.initialPhi.isNull());

    auto initialPhi = imageViewFromQImage(config_.initialPhi);

    fluvel_ip::ContourData initialCD(initialPhi, config_.contourConfig.connectivity);

    bool is_rgb = (processedImage_.format() != QImage::Format_Grayscale8);

    const auto processedImg = imageViewFromQImage(processedImage_);

    if (is_rgb)
    {
        activeContour_ = std::make_unique<fluvel_ip::ActiveContour>(
            std::move(initialCD),
            std::make_unique<fluvel_ip::RegionColorSpeedModel>(config_.contourConfig.regionParams),
            config_.contourConfig.contourParams);
    }
    else
    {
        activeContour_ = std::make_unique<fluvel_ip::ActiveContour>(
            std::move(initialCD),
            std::make_unique<fluvel_ip::RegionGraySpeedModel>(config_.contourConfig.regionParams),
            config_.contourConfig.contourParams);
    }

    activeContour_->update(processedImg);

    setState(WorkerState::Ready);
    initialShown_ = false;
}

void ActiveContourWorker::finish()
{
    workerTimer_->stop();
    setState(WorkerState::Finished);
}

void ActiveContourWorker::finalizeAndPrepareNextRun()
{
    finish();

    updateDiagnostics();
    emitContour();

    initializeActiveContour();
}

void ActiveContourWorker::suspend()
{
    if (state_ == WorkerState::Running)
    {
        workerTimer_->stop();
        setState(WorkerState::Suspended);

        updateDiagnostics();
        emitContour();
    }
}

void ActiveContourWorker::resume()
{
    if (state_ == WorkerState::Suspended || state_ == WorkerState::Ready)
        start();
}

void ActiveContourWorker::onTimeout()
{
    assert(activeContour_ != nullptr);
    assert(state_ == WorkerState::Running);

    fluvel_ip::ElapsedTimer timeSliceBudget;
    timeSliceBudget.start();

    while (timeSliceBudget.elapsedLessThan(timeSliceMs_))
    {
        if (stepOnceAlgo())
        {
            finalizeAndPrepareNextRun();
            return;
        }
    }

    if (mode_ == RunMode::Interactive)
    {
        updateDiagnostics();
        emitContour();
    }
}

void ActiveContourWorker::updateDiagnostics()
{
    if (!activeContour_)
        return;

    fluvel_ip::ContourDiagnostics diag;

    if (isMeasuring_)
        diag.elapsedSec = measurementTimer_.elapsedSec();
    else
        diag.elapsedSec = 0.0;

    activeContour_->fillDiagnostics(diag);

    emit diagnosticsUpdated(diag);
}

void ActiveContourWorker::emitContour()
{
    if (!activeContour_)
        return;

    emit contourUpdated(activeContour_->export_l_out(), activeContour_->export_l_in());
}

void ActiveContourWorker::setMode(RunMode mode)
{
    mode_ = mode;

    if (mode_ == RunMode::Interactive)
        timeSliceMs_ = kTimeSliceInteractiveMs;
    else if (mode_ == RunMode::Converge)
        timeSliceMs_ = kTimeSliceConvergeMs;
}

void ActiveContourWorker::setState(WorkerState state)
{
    state_ = state;
    emit stateChanged(state);
}

void ActiveContourWorker::resetMeasurement()
{
    isMeasuring_ = false;
}

} // namespace fluvel_app
