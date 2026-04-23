// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "elapsed_timer.hpp"
#include "frame_data.hpp"

#include <QMutex>
#include <QtGlobal>

namespace fluvel_app
{

class FrameStatsView
{
public:
    struct Snapshot
    {
        double capturedFps{0.0};
        double processedFps{0.0};
        double displayedFps{0.0};
        double dropRate{0.0};
        double avgLatencyDisplayMs{0.0};
        double maxLatencyDisplayMs{0.0};
        double avgLatencyProcMs{0.0};
        double avgContourSize{0.0};
    };

    FrameStatsView();

    // événements
    void frameCaptured();
    void frameProcessed(quint64 contourSize);
    void frameDisplayed(const FrameTimestamps& ts);

    // lecture stable (fenêtre ~1s)
    Snapshot snapshot();

    void reset();

private:
    void updateWindowLocked();

    QMutex mutex_;

    // compteurs fenêtre courante
    quint64 capturedFrames_{0};
    quint64 processedFrames_{0};
    quint64 displayedFrames_{0};
    quint64 droppedFrames_{0};

    quint64 contourSizeSum_{0};

    // latence fenêtre
    double latencySumDisplayMs_{0.0};
    double latencyMaxDisplayMs_{0.0};
    double latencySumProcMs_{0.0};

    fluvel_ip::ElapsedTimer windowTimer_;
    static constexpr std::chrono::milliseconds kWindowMs{1000}; // 1 seconde

    // snapshot précédent (pour affichage stable)
    Snapshot lastSnapshot_{};
};

} // namespace fluvel_app
