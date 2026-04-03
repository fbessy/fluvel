// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "frame_stats_view.hpp"
#include <QMutexLocker>

namespace fluvel_app
{

FrameStatsView::FrameStatsView()
{
    reset();
}

void FrameStatsView::reset()
{
    QMutexLocker lock(&mutex_);

    capturedFrames_ = 0;
    processedFrames_ = 0;
    displayedFrames_ = 0;
    droppedFrames_ = 0;

    contourSizeSum_ = 0;

    latencySumDisplayMs_ = 0.0;
    latencyMaxDisplayMs_ = 0.0;
    latencySumProcMs_ = 0.0;

    windowTimer_.restart();

    lastSnapshot_ = {};
}

void FrameStatsView::frameCaptured()
{
    QMutexLocker lock(&mutex_);

    ++capturedFrames_;
    updateWindowLocked();
}

void FrameStatsView::frameProcessed(quint64 contourSize)
{
    QMutexLocker lock(&mutex_);

    contourSizeSum_ += contourSize;
    ++processedFrames_;
}

void FrameStatsView::frameDisplayed(const FrameTimestamps& ts)
{
    QMutexLocker lock(&mutex_);

    ++displayedFrames_;

    // Latences fiables (basées sur la frame)
    double latencyDisplayMs = double(ts.displayTimestampNs - ts.receiveTimestampNs) * 1e-6;

    double latencyProcMs = double(ts.processTimestampNs - ts.receiveTimestampNs) * 1e-6;

    latencySumDisplayMs_ += latencyDisplayMs;
    latencyMaxDisplayMs_ = std::max(latencyMaxDisplayMs_, latencyDisplayMs);

    latencySumProcMs_ += latencyProcMs;

    updateWindowLocked();
}

FrameStatsView::Snapshot FrameStatsView::snapshot()
{
    QMutexLocker lock(&mutex_);
    return lastSnapshot_;
}

void FrameStatsView::updateWindowLocked()
{
    if (windowTimer_.elapsedLessThan(kWindowMs))
        return;

    double seconds = windowTimer_.elapsedSec();

    Snapshot snap;

    // FPS
    snap.capturedFps = double(capturedFrames_) / seconds;
    snap.processedFps = double(processedFrames_) / seconds;
    snap.displayedFps = double(displayedFrames_) / seconds;

    // Drop
    droppedFrames_ =
        (capturedFrames_ > displayedFrames_) ? (capturedFrames_ - displayedFrames_) : 0;

    snap.dropRate = capturedFrames_ > 0 ? double(droppedFrames_) / double(capturedFrames_) : 0.0;

    // Latence display
    snap.avgLatencyDisplayMs =
        displayedFrames_ > 0 ? latencySumDisplayMs_ / double(displayedFrames_) : 0.0;

    snap.maxLatencyDisplayMs = latencyMaxDisplayMs_;

    // Latence proc
    snap.avgLatencyProcMs =
        displayedFrames_ > 0 ? latencySumProcMs_ / double(displayedFrames_) : 0.0;

    // Contour
    snap.avgContourSize =
        processedFrames_ > 0 ? double(contourSizeSum_) / double(processedFrames_) : 0.0;

    lastSnapshot_ = snap;

    // Reset fenêtre
    capturedFrames_ = 0;
    processedFrames_ = 0;
    displayedFrames_ = 0;
    droppedFrames_ = 0;

    contourSizeSum_ = 0;

    latencySumDisplayMs_ = 0.0;
    latencyMaxDisplayMs_ = 0.0;
    latencySumProcMs_ = 0.0;

    windowTimer_.restart();
}

} // namespace fluvel_app
