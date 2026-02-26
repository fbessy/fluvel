// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "active_contour_worker.hpp"

#include "region_ac.hpp"
#include "region_color_ac.hpp"

#include "image_adapters.hpp"
#include "image_span.hpp"

#include <QDebug>
#include <QElapsedTimer>
#include <QTimer>

#include <cassert>

namespace ofeli_app
{

constexpr int kWorkerPeriodMs = 16;
constexpr qint64 kTimeSliceInteractiveMs = 10;
constexpr qint64 kTimeSliceConvergeMs = 15;

ActiveContourWorker::ActiveContourWorker()
    : QObject(nullptr)
    ,

    workerTimer_(new QTimer(this))
    , timeSliceMs_(kTimeSliceInteractiveMs)
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

    // if there is at least one random operation,
    // preprocessing is done at each restart
    if (fc.hasProcessing() && (fc.has_gaussian_noise || fc.has_salt_noise || fc.has_speckle_noise))
    {
        applyProcessing();
    }

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
    assert(ac_ != nullptr);

    if (ac_->isStopped())
    {
        initializeActiveContour();
    }
    else
    {
        if (ac_->isFirstIteration())
        {
            isMeasuring_ = true;
            measurementStartTime_ = clock_type::now();
        }

        ac_->step();
    }

    return ac_->isStopped();
}

void ActiveContourWorker::applyProcessing()
{
    if (image_.isNull())
        return;

    workerTimer_->stop();
    setState(WorkerState::Initializing);

    processedImage_ = image_;

    // float elapsed_time;
    // std::clock_t start_time, stop_time;

    QImage img;

    int channelsNbr = 3;

    if (image_.format() == QImage::Format_Grayscale8)
    {
        img = image_;
        channelsNbr = 1;
    }
    else if (image_.format() == QImage::Format_RGB32)
    {
        img = image_.convertToFormat(QImage::Format_RGB888);
        channelsNbr = 3;
    }

    if (img.isNull())
        return;

    const int width = img.width();

    if (img.bytesPerLine() != static_cast<qsizetype>(width * channelsNbr))
        return;

    const qsizetype stride = img.bytesPerLine();

    const int bytesPerPixel = static_cast<int>(stride / width);

    ofeli_ip::Filters filters(img.constBits(), width, img.height(), bytesPerPixel);

    // start_time = std::clock();

    const auto& fc = config_.processing;

    if (fc.hasProcessing())
    {
        if (fc.has_gaussian_noise)
        {
            filters.gaussian_white_noise(fc.std_noise);
        }
        if (fc.has_salt_noise)
        {
            filters.salt_pepper_noise(fc.proba_noise);
        }
        if (fc.has_speckle_noise)
        {
            filters.speckle(fc.std_speckle_noise);
        }

        if (fc.has_mean_filt)
        {
            filters.mean_filtering(fc.kernel_mean_length);
        }
        if (fc.has_gaussian_filt)
        {
            filters.gaussian_filtering(fc.kernel_gaussian_length, fc.sigma);
        }

        if (fc.has_median_filt)
        {
            if (fc.has_O1_algo)
            {
                filters.median_filtering_o1(fc.kernel_median_length);
            }
            else
            {
                filters.median_filtering_oNlogN(fc.kernel_median_length);
            }
        }
        if (fc.has_aniso_diff)
        {
            filters.anisotropic_diffusion(fc.max_itera, fc.lambda, fc.kappa, fc.aniso_option);
        }

        if (fc.has_open_filt)
        {
            if (fc.has_O1_morpho)
            {
                filters.opening_o1(fc.kernel_open_length);
            }
            else
            {
                filters.opening(fc.kernel_open_length);
            }
        }

        if (fc.has_close_filt)
        {
            if (fc.has_O1_morpho)
            {
                filters.closing_o1(fc.kernel_close_length);
            }
            else
            {
                filters.closing(fc.kernel_close_length);
            }
        }

        if (fc.has_top_hat_filt)
        {
            if (fc.is_white_top_hat)
            {
                if (fc.has_O1_morpho)
                {
                    filters.white_top_hat_o1(fc.kernel_tophat_length);
                }
                else
                {
                    filters.white_top_hat(fc.kernel_tophat_length);
                }
            }
            else
            {
                if (fc.has_O1_morpho)
                {
                    filters.black_top_hat_o1(fc.kernel_tophat_length);
                }
                else
                {
                    filters.black_top_hat(fc.kernel_tophat_length);
                }
            }
        }

        // stop_time = std::clock();
        // elapsed_time = float(stop_time - start_time) / float(CLOCKS_PER_SEC);

        if (img.format() == QImage::Format_RGB888)
        {
            processedImage_ =
                QImage(filters.get_filtered(), img.width(), img.height(), QImage::Format_RGB888)
                    .convertToFormat(QImage::Format_RGB32);
        }
        else if (img.format() == QImage::Format_Grayscale8)
        {
            processedImage_ =
                QImage(filters.get_filtered(), img.width(), img.height(), QImage::Format_Grayscale8)
                    .copy();
        }
    }

    if (!processedImage_.isNull())
        emit processedImageReady(processedImage_);

    // return elapsed_time;
}

void ActiveContourWorker::initialize(const QImage& image, const ImageComputeConfig& config)
{
    workerTimer_->stop();
    setState(WorkerState::Initializing);

    image_ = image;
    config_ = config;

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

    ac_.reset();
    resetMeasurement();

    assert(processedImage_.size() == config_.initialPhi.size());

    if (processedImage_.size() != config_.initialPhi.size())
    {
        config_.initialPhi = config_.initialPhi.scaled(
            processedImage_.size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }

    assert(!config_.initialPhi.isNull());

    auto initialPhi = image_span_from_qimage(config_.initialPhi);

    ofeli_ip::ContourData initialCD(initialPhi, config_.algo.connectivity);

    bool is_rgb = (processedImage_.format() != QImage::Format_Grayscale8);

    const auto processedImg = image_span_from_qimage(processedImage_);

    if (is_rgb)
    {
        ac_ = std::make_unique<ofeli_ip::RegionColorAc>(
            processedImg, std::move(initialCD), config_.algo.acConfig, config_.algo.regionAcConfig);
    }
    else
    {
        ac_ = std::make_unique<ofeli_ip::RegionAc>(
            processedImg, std::move(initialCD), config_.algo.acConfig, config_.algo.regionAcConfig);
    }

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
    assert(ac_ != nullptr);
    assert(state_ == WorkerState::Running);

    QElapsedTimer timeSliceBudgetMs;
    timeSliceBudgetMs.start();

    while (timeSliceBudgetMs.elapsed() < timeSliceMs_)
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
    if (!ac_)
        return;

    ofeli_ip::ContourDiagnostics diag;

    if (isMeasuring_)
    {
        auto endTime = clock_type::now();
        diag.elapsedSec = std::chrono::duration<double>(endTime - measurementStartTime_).count();
    }
    else
    {
        diag.elapsedSec = 0.0;
    }

    ac_->fillDiagnostics(diag);

    emit diagnosticsUpdated(diag);
}

void ActiveContourWorker::emitContour()
{
    if (!ac_)
        return;

    emit contourUpdated(ac_->export_l_out(), ac_->export_l_in());
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

void ActiveContourWorker::setAlgoConfig(const ImageComputeConfig& config)
{
    config_ = config;

    applyProcessing();
    initializeActiveContour();

    if (state_ != WorkerState::Ready)
        return;

    updateDiagnostics();
    emitContour();

    initialShown_ = true;
}

void ActiveContourWorker::resetMeasurement()
{
    isMeasuring_ = false;
    measurementStartTime_ = clock_type::time_point{};
}

} // namespace ofeli_app
