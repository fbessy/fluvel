// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QMutex>
#include <QtGlobal>

namespace fluvel_app
{

class FrameStatsView
{
public:
    struct Snapshot
    {
        double capturedFps = 0.0;
        double processedFps = 0.0;
        double displayedFps = 0.0;
        double dropRate = 0.0;
        double avgLatencyMs = 0.0;
        double maxLatencyMs = 0.0;
        double avgContourSize = 0.0;
    };

    FrameStatsView();

    // événements
    void frameCaptured(qint64 receiveTsNs);
    void frameProcessed(quint64 contourSize);
    void frameDisplayed(qint64 receiveTsNs, qint64 displayTsNs);

    // lecture stable (fenêtre ~1s)
    Snapshot snapshot();

    void reset();

private:
    void updateWindowLocked(qint64 nowNs);

private:
    QMutex mutex_;

    // compteurs fenêtre courante
    quint64 capturedFrames_;
    quint64 processedFrames_;
    quint64 displayedFrames_;
    quint64 droppedFrames_;

    quint64 contourSizeSum_;

    // latence fenêtre
    double latencySumMs_;
    double latencyMaxMs_;

    // timestamps
    qint64 windowStartNs_;

    // snapshot précédent (pour affichage stable)
    Snapshot lastSnapshot_;
};

} // namespace fluvel_app
