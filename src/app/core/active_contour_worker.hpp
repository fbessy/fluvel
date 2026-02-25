// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QImage>
#include <QMutex>
#include <QObject>
#include <QTimer>

#include "active_contour.hpp"
#include "algo_stats.hpp"
#include "common_settings.hpp"

namespace ofeli_app
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

    void finish();

    AlgoStats currentStats() const;

    void setAlgoConfig(const ImageComputeConfig& config);

signals:
    void processedImageReady(const QImage& img);
    void contourUpdated(const ofeli_ip::ExportedContour& l_out,
                        const ofeli_ip::ExportedContour& l_in);
    void stateChanged(ofeli_app::WorkerState state);

private slots:
    void onTimeout();

private:
    void emitContour();
    void updateStats();

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

    WorkerState state_{WorkerState::Uninitialized};
    RunMode mode_{RunMode::Interactive};
    QTimer* timer_;
    std::unique_ptr<ofeli_ip::ActiveContour> ac_;

    mutable QMutex statsMutex_;
    AlgoStats currentStats_;

    QImage image_;
    QImage processedImage_;

    qint64 timeSlice_ms_;
    bool initialShown_;

    ImageComputeConfig config_;
};

} // namespace ofeli_app
