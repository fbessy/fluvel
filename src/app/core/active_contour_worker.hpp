// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour.hpp"
#include "common_settings.hpp"
#include "contour_diagnostics.hpp"

#include <QObject>
#include <QImage>
#include <QTimer>
#include <QMutex>

#include <chrono>

namespace fluvel_app
{

using clock_type = std::chrono::steady_clock;

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

    void finish();

    void setAlgoConfig(const ImageComputeConfig& config);

signals:
    void processedImageReady(const QImage& img);
    void contourUpdated(const fluvel_ip::ExportedContour& l_out,
                        const fluvel_ip::ExportedContour& l_in);
    void stateChanged(fluvel_app::WorkerState state);
    void diagnosticsUpdated(fluvel_ip::ContourDiagnostics diag);

private slots:
    void onTimeout();

private:
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
    static constexpr qint64 kTimeSliceConvergeMs = 15;
    static constexpr qint64 kTimeSliceInteractiveMs = 10;
    qint64 timeSliceMs_ = kTimeSliceInteractiveMs;

    bool initialShown_ = false;

    ImageComputeConfig config_;

    fluvel_ip::ContourDiagnostics diag_;
    clock_type::time_point measurementStartTime_;
    bool isMeasuring_ = false;
};

} // namespace fluvel_app
