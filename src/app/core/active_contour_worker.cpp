// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "active_contour_worker.hpp"

#include "region_ac.hpp"
#include "region_color_ac.hpp"
#include "speed_model.hpp"

#include "filters.hpp"
#include "image_adapters.hpp"
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
    if (image_.isNull())
        return;

#ifdef FLUVEL_DEBUG
    qDebug() << __FILE__ << " applyProcessing() " << __LINE__ << __func__;
#endif

    workerTimer_->stop();
    setState(WorkerState::Initializing);

    //--------------------------------------------------
    // 1. Conversion FORMAT STRICT attendu par l'algo
    //--------------------------------------------------
    QImage img;
    int channelsNbr = 0;

    if (image_.format() == QImage::Format_Grayscale8)
    {
        img = image_;
        channelsNbr = 1;
    }
    else
    {
        // 👉 on force TOUJOURS RGB888 (clé du problème)
        img = image_.convertToFormat(QImage::Format_RGB888);
        channelsNbr = 3;
    }

    if (img.isNull())
        return;

    const int width = img.width();
    const int height = img.height();

    //--------------------------------------------------
    // 2. Garantir buffer PACKED (stride = width * channels)
    //--------------------------------------------------
    const unsigned char* dataPtr = nullptr;
    std::vector<unsigned char> packedBuffer;

    const qsizetype expectedStride = static_cast<qsizetype>(width * channelsNbr);

    if (img.bytesPerLine() == expectedStride)
    {
        dataPtr = img.constBits(); // ✅ const correct
    }
    else
    {
        packedBuffer.resize(static_cast<std::size_t>(width * height * channelsNbr));

        for (int y = 0; y < height; ++y)
        {
            const uchar* src = img.constScanLine(y);
            uchar* dst = packedBuffer.data() + y * width * channelsNbr;

            memcpy(dst, src, static_cast<std::size_t>(width * channelsNbr));
        }

        dataPtr = packedBuffer.data(); // implicit const OK
    }

    //--------------------------------------------------
    // 3. Pipeline filtres (buffer TOUJOURS valide ici)
    //--------------------------------------------------
    const auto& fc = config_.processing;

    if (fc.hasProcessing())
    {
        fluvel_ip::Filters filters(dataPtr, width, height, channelsNbr);

        //--------------------------------------------------
        // Bruits
        //--------------------------------------------------
        if (fc.has_gaussian_noise)
            filters.gaussian_white_noise(fc.std_noise);

        if (fc.has_salt_noise)
            filters.impulsive_noise(fc.proba_noise);

        if (fc.has_speckle_noise)
            filters.speckle(fc.std_speckle_noise);

        //--------------------------------------------------
        // Filtres linéaires
        //--------------------------------------------------
        if (fc.has_mean_filt)
            filters.mean_filtering(fc.kernel_mean_length);

        if (fc.has_gaussian_filt)
            filters.gaussian_filtering(fc.kernel_gaussian_length, fc.sigma);

        //--------------------------------------------------
        // Médian
        //--------------------------------------------------
        if (fc.has_median_filt)
        {
            if (fc.has_O1_algo)
                filters.median_filtering_o1(fc.kernel_median_length);
            else
                filters.median_filtering_oNlogN(fc.kernel_median_length);
        }

        //--------------------------------------------------
        // Diffusion anisotrope
        //--------------------------------------------------
        if (fc.has_aniso_diff)
        {
            // filters.anisotropic_diffusion(fc.max_itera, fc.lambda, fc.kappa, fc.aniso_option);
        }

        //--------------------------------------------------
        // Morphologie
        //--------------------------------------------------
        if (fc.has_open_filt)
        {
            if (fc.has_O1_morpho)
                filters.opening_o1(fc.kernel_open_length);
            else
                filters.opening(fc.kernel_open_length);
        }

        if (fc.has_close_filt)
        {
            if (fc.has_O1_morpho)
                filters.closing_o1(fc.kernel_close_length);
            else
                filters.closing(fc.kernel_close_length);
        }

        if (fc.has_top_hat_filt)
        {
            if (fc.is_white_top_hat)
            {
                if (fc.has_O1_morpho)
                    filters.white_top_hat_o1(fc.kernel_tophat_length);
                else
                    filters.white_top_hat(fc.kernel_tophat_length);
            }
            else
            {
                if (fc.has_O1_morpho)
                    filters.black_top_hat_o1(fc.kernel_tophat_length);
                else
                    filters.black_top_hat(fc.kernel_tophat_length);
            }
        }

        //--------------------------------------------------
        // 4. Reconstruction QImage (SAFE)
        //--------------------------------------------------
        const unsigned char* out = filters.get_filtered();

        if (channelsNbr == 3)
        {
            QImage tmp(out, width, height, width * channelsNbr, QImage::Format_RGB888);
            processedImage_ = tmp.convertToFormat(QImage::Format_RGB32).copy();
        }
        else
        {
            QImage tmp(out, width, height, width * channelsNbr, QImage::Format_Grayscale8);
            processedImage_ = tmp.copy();
        }
    }
    else
    {
        // 👉 pas de processing → on garde l'image convertie
        processedImage_ = img;
    }

    //--------------------------------------------------
    // 5. Emit
    //--------------------------------------------------
    if (!processedImage_.isNull())
        emit processedImageReady(processedImage_);
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

    fluvel_ip::ContourData initialCD(initialPhi, config_.algo.connectivity);

    bool is_rgb = (processedImage_.format() != QImage::Format_Grayscale8);

    const auto processedImg = imageViewFromQImage(processedImage_);

    if (is_rgb)
    {
        activeContour_ = std::make_unique<fluvel_ip::ActiveContour>(
            std::move(initialCD),
            std::make_unique<fluvel_ip::RegionColorSpeedModel>(config_.algo.regionAcConfig),
            config_.algo.acConfig);
    }
    else
    {
        activeContour_ = std::make_unique<fluvel_ip::ActiveContour>(
            std::move(initialCD),
            std::make_unique<fluvel_ip::RegionGraySpeedModel>(config_.algo.regionAcConfig),
            config_.algo.acConfig);
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
