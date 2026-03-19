// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "frame_stats_view.hpp"

namespace fluvel_app
{

static constexpr qint64 kWindowNs = 1'000'000'000LL; // 1 seconde

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

    latencySumMs_ = 0.0;
    latencyMaxMs_ = 0.0;

    windowStartNs_ = 0;
    lastSnapshot_ = {};
}

void FrameStatsView::frameCaptured(qint64 receiveTsNs)
{
    QMutexLocker lock(&mutex_);

    if (windowStartNs_ == 0)
        windowStartNs_ = receiveTsNs;

    ++capturedFrames_;
    updateWindowLocked(receiveTsNs);
}

void FrameStatsView::frameProcessed(quint64 contourSize)
{
    QMutexLocker lock(&mutex_);

    contourSizeSum_ += contourSize;
    ++processedFrames_;
}

void FrameStatsView::frameDisplayed(qint64 receiveTsNs, qint64 displayTsNs)
{
    QMutexLocker lock(&mutex_);

    ++displayedFrames_;

    // latence affichage - réception
    double latencyMs = double(displayTsNs - receiveTsNs) * 1e-6;

    latencySumMs_ += latencyMs;
    latencyMaxMs_ = std::max(latencyMaxMs_, latencyMs);

    updateWindowLocked(displayTsNs);
}

FrameStatsView::Snapshot FrameStatsView::snapshot()
{
    QMutexLocker lock(&mutex_);
    return lastSnapshot_;
}

void FrameStatsView::updateWindowLocked(qint64 nowNs)
{
    qint64 elapsed = nowNs - windowStartNs_;
    if (elapsed < kWindowNs)
        return;

    double seconds = double(elapsed) * 1e-9;

    Snapshot snap;

    snap.capturedFps = double(capturedFrames_) / seconds;
    snap.processedFps = double(processedFrames_) / seconds;
    snap.displayedFps = double(displayedFrames_) / seconds;

    droppedFrames_ =
        (capturedFrames_ > displayedFrames_) ? (capturedFrames_ - displayedFrames_) : 0;

    snap.dropRate = capturedFrames_ > 0 ? double(droppedFrames_) / double(capturedFrames_) : 0.0;

    snap.avgLatencyMs = displayedFrames_ > 0 ? latencySumMs_ / double(displayedFrames_) : 0.0;

    snap.maxLatencyMs = latencyMaxMs_;

    snap.avgContourSize = double(contourSizeSum_) / double(processedFrames_);

    lastSnapshot_ = snap;

    // reset fenêtre
    capturedFrames_ = 0;
    processedFrames_ = 0;
    displayedFrames_ = 0;
    droppedFrames_ = 0;
    contourSizeSum_ = 0;

    latencySumMs_ = 0.0;
    latencyMaxMs_ = 0.0;

    windowStartNs_ = nowNs;
}

} // namespace fluvel_app
