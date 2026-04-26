// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QImage>
#include <QMutex>
#include <QObject>
#include <QTimer>

#ifndef Q_MOC_RUN

#include "active_contour.hpp"
#include "application_settings_types.hpp"
#include "contour_diagnostics.hpp"
#include "elapsed_timer.hpp"
#include "image_owner.hpp"
#include <chrono>

#endif

namespace fluvel_app
{

/**
 * @brief Execution mode of the worker.
 *
 * - Interactive: periodic updates with UI refresh.
 * - Converge   : runs as fast as possible until completion, without UI updates.
 */
enum class RunMode
{
    Interactive, ///< UI updates enabled
    Converge     ///< Full-speed execution without intermediate UI updates
};

/**
 * @brief Internal state of the ActiveContourWorker.
 */
enum class WorkerState
{
    Uninitialized, ///< Worker has not been initialized
    Initializing,  ///< Preparing active contour
    Ready,         ///< Ready to start processing
    Suspended,     ///< Paused state
    Running,       ///< Processing is ongoing (timer active)
    Finished       ///< Processing finished and reset prepared for next run
};

/**
 * @brief Qt worker managing the lifecycle and execution of an active contour.
 *
 * This class acts as a bridge between the UI layer and the core active contour
 * algorithm. It handles initialization, execution control (start, pause, step),
 * and result propagation through Qt signals.
 *
 * Execution is driven by a QTimer in interactive mode, allowing periodic updates
 * suitable for UI rendering. In converge mode, the algorithm runs continuously
 * until completion without intermediate updates.
 *
 * Responsibilities:
 * - Manage active contour lifecycle
 * - Control execution flow (interactive vs converge)
 * - Emit processed images and contour updates
 * - Provide diagnostics and performance measurements
 *
 * @note This class is intended to be used from the Qt event loop.
 */
class ActiveContourWorker : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct the worker.
     */
    ActiveContourWorker();

    /**
     * @brief Initialize the worker with an input image and configuration.
     *
     * @param image Input image.
     * @param config Processing configuration.
     */
    void initialize(const QImage& image, const ImageComputeConfig& config);

    /**
     * @brief Restart processing from scratch.
     *
     * Resets internal state and starts execution.
     */
    void restart();

    /**
     * @brief Toggle pause/resume state.
     */
    void togglePause();

    /**
     * @brief Execute a single iteration of the algorithm.
     */
    void step();

    /**
     * @brief Run the algorithm until convergence.
     *
     * Disables intermediate UI updates and emits only the final result.
     */
    void converge();

signals:
    /**
     * @brief Emitted when a processed image is available.
     */
    void processedImageReady(const QImage& img);

    /**
     * @brief Emitted when contour geometry is updated.
     */
    void contourUpdated(const fluvel_ip::ExportedContour& outerContour,
                        const fluvel_ip::ExportedContour& innerContour);

    /**
     * @brief Emitted when the worker state changes.
     */
    void stateChanged(fluvel_app::WorkerState state);

    /**
     * @brief Emitted when diagnostics are updated.
     */
    void diagnosticsUpdated(fluvel_ip::ContourDiagnostics diag);

private:
    /**
     * @brief Timer callback driving the execution loop.
     */
    void onTimeout();

    /// Emit current contour data.
    void emitContour();

    /// Update diagnostics structure.
    void updateDiagnostics();

    /// Suspend execution.
    void suspend();

    /// Resume execution.
    void resume();

    /// Start execution.
    void start();

    /// Perform one processing step (wrapper).
    void performStep();

    /**
     * @brief Execute one algorithm step.
     * @return true if the algorithm should continue, false if finished.
     */
    bool stepOnceAlgo();

    /// Apply pre-processing pipeline.
    void applyProcessing();

    /// Initialize the active contour instance.
    void initializeActiveContour();

    /// Finalize current run and prepare next execution.
    void finalizeAndPrepareNextRun();

    /// Finish execution.
    void finish();

    /// Set execution mode.
    void setMode(RunMode mode);

    /// Set worker state and emit signal.
    void setState(WorkerState state);

    /// Reset performance measurement.
    void resetMeasurement();

    WorkerState state_{WorkerState::Uninitialized};
    RunMode mode_{RunMode::Interactive};
    QTimer* workerTimer_;
    std::unique_ptr<fluvel_ip::ActiveContour> activeContour_;

    QImage image_;
    QImage processedImage_;
    fluvel_ip::ImageOwner processedOwner_;

    /// Period of the worker QTimer in ms (~16 ms ≈ 60 FPS target).
    static constexpr int kWorkerPeriodMs{16};

    /// Time slice for converge mode.
    static constexpr std::chrono::milliseconds kTimeSliceConvergeMs{15};

    /// Time slice for interactive mode.
    static constexpr std::chrono::milliseconds kTimeSliceInteractiveMs{10};

    static_assert(kTimeSliceConvergeMs.count() <= kWorkerPeriodMs,
                  "Converge time slice must be <= worker period");

    static_assert(kTimeSliceInteractiveMs.count() <= kWorkerPeriodMs,
                  "Interactive time slice must be <= worker period");

    std::chrono::milliseconds timeSliceMs_{kTimeSliceInteractiveMs};

    bool initialShown_{false};

    ImageComputeConfig config_;

    fluvel_ip::ContourDiagnostics diag_;
    fluvel_ip::ElapsedTimer measurementTimer_;
    bool isMeasuring_{false};

    bool processingDirty_{true};
};

} // namespace fluvel_app
