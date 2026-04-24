// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour.hpp"
#include "application_settings_types.hpp"
#include "contour_diagnostics.hpp"
#include "elapsed_timer.hpp"

#include <QImage>
#include <QMutex>
#include <QObject>
#include <QTimer>

#include <chrono>

namespace fluvel_app
{

enum class RunMode
{
    Interactive, // UI updates
    Converge     // full speed without freeze, no UI updates
};

enum class WorkerState
{
    Uninitialized,
    Initializing, // prepare active contour
    Ready,
    Suspended,
    Running,  // timer is active
    Finished, // atomic state because the active contour is reset
              // to prepare the next run
};

class ActiveContourWorker : public QObject
{
    Q_OBJECT

public:
    ActiveContourWorker();

    void initialize(const QImage& image, const ImageComputeConfig& config);

    void restart();     // reset + start
    void togglePause(); // suspend / resume
    void step();        // one iteration
    void converge();    // converge to the final state and
                        // display only the final result

signals:
    void processedImageReady(const QImage& img);
    void contourUpdated(const fluvel_ip::ExportedContour& outerContour,
                        const fluvel_ip::ExportedContour& innerContour);
    void stateChanged(fluvel_app::WorkerState state);
    void diagnosticsUpdated(fluvel_ip::ContourDiagnostics diag);

private:
    void onTimeout();

    void emitContour();
    void updateDiagnostics();

    void suspend();
    void resume();
    void start();
    void performStep();
    bool stepOnceAlgo();

    void applyProcessing();
    void initializeActiveContour();
    void finalizeAndPrepareNextRun();

    void finish();

    void setMode(RunMode mode);
    void setState(WorkerState state);

    void resetMeasurement();

    WorkerState state_{WorkerState::Uninitialized};
    RunMode mode_{RunMode::Interactive};
    QTimer* workerTimer_;
    std::unique_ptr<fluvel_ip::ActiveContour> activeContour_;

    QImage image_;
    QImage processedImage_;

    static constexpr int kWorkerPeriodMs = 16;

    static constexpr std::chrono::milliseconds kTimeSliceConvergeMs{15};
    static constexpr std::chrono::milliseconds kTimeSliceInteractiveMs{10};

    std::chrono::milliseconds timeSliceMs_{kTimeSliceInteractiveMs};

    bool initialShown_{false};

    ImageComputeConfig config_;

    fluvel_ip::ContourDiagnostics diag_;
    fluvel_ip::ElapsedTimer measurementTimer_;
    bool isMeasuring_{false};

    bool processingDirty_{true};
};

} // namespace fluvel_app
