// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "frame_stats_collector.hpp"
#include <QMutexLocker>

namespace fluvel
{

FrameStatsCollector::FrameStatsCollector()
{
    reset();
}

void FrameStatsCollector::reset()
{
    QMutexLocker lock(&mutex_);

    receivedFrames_ = 0;
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

void FrameStatsCollector::frameReceived()
{
    QMutexLocker lock(&mutex_);

    ++receivedFrames_;
    updateWindowLocked();
}

void FrameStatsCollector::frameProcessed(quint64 contourSize)
{
    QMutexLocker lock(&mutex_);

    contourSizeSum_ += contourSize;
    ++processedFrames_;
}

void FrameStatsCollector::frameDisplayed(const FrameTimestamps& ts)
{
    QMutexLocker lock(&mutex_);

    ++displayedFrames_;

    // Frame-based latency measurements
    double latencyDisplayMs = double(ts.displayTimestampNs - ts.receiveTimestampNs) * 1e-6;

    double latencyProcMs = double(ts.processTimestampNs - ts.receiveTimestampNs) * 1e-6;

    latencySumDisplayMs_ += latencyDisplayMs;
    latencyMaxDisplayMs_ = std::max(latencyMaxDisplayMs_, latencyDisplayMs);

    latencySumProcMs_ += latencyProcMs;

    updateWindowLocked();
}

FrameStatsCollector::Snapshot FrameStatsCollector::snapshot()
{
    QMutexLocker lock(&mutex_);
    return lastSnapshot_;
}

void FrameStatsCollector::updateWindowLocked()
{
    if (windowTimer_.elapsedLessThan(kWindowMs))
        return;

    double seconds = windowTimer_.elapsedSec();

    Snapshot snap;

    // FPS
    snap.receivedFps = double(receivedFrames_) / seconds;
    snap.processedFps = double(processedFrames_) / seconds;
    snap.displayedFps = double(displayedFrames_) / seconds;

    // Drop
    droppedFrames_ =
        (receivedFrames_ > displayedFrames_) ? (receivedFrames_ - displayedFrames_) : 0;

    snap.dropRate = receivedFrames_ > 0 ? double(droppedFrames_) / double(receivedFrames_) : 0.0;

    // Display latency
    snap.avgLatencyDisplayMs =
        displayedFrames_ > 0 ? latencySumDisplayMs_ / double(displayedFrames_) : 0.0;

    snap.maxLatencyDisplayMs = latencyMaxDisplayMs_;

    // Processing latency
    snap.avgLatencyProcMs =
        displayedFrames_ > 0 ? latencySumProcMs_ / double(displayedFrames_) : 0.0;

    // Contour statistics
    snap.avgContourSize =
        processedFrames_ > 0 ? double(contourSizeSum_) / double(processedFrames_) : 0.0;

    lastSnapshot_ = snap;

    // Window reset
    receivedFrames_ = 0;
    processedFrames_ = 0;
    displayedFrames_ = 0;
    droppedFrames_ = 0;

    contourSizeSum_ = 0;

    latencySumDisplayMs_ = 0.0;
    latencyMaxDisplayMs_ = 0.0;
    latencySumProcMs_ = 0.0;

    windowTimer_.restart();
}

} // namespace fluvel
