#include "frame_stats.hpp"

namespace ofeli_gui {

static constexpr qint64 WINDOW_NS = 1'000'000'000LL; // 1 seconde

FrameStats::FrameStats()
{
    reset();
}

void FrameStats::reset()
{
    QMutexLocker lock(&mutex);

    inputFrames = 0;
    processedFrames = 0;
    displayedFrames = 0;
    droppedFrames = 0;

    latencySumMs = 0.0;
    latencyMaxMs = 0.0;

    windowStartNs = 0;
    lastSnapshot = {};
}

void FrameStats::frameReceived(qint64 recvTsNs)
{
    QMutexLocker lock(&mutex);

    if (windowStartNs == 0)
        windowStartNs = recvTsNs;

    inputFrames++;
    updateWindowLocked(recvTsNs);
}

void FrameStats::frameProcessed()
{
    QMutexLocker lock(&mutex);
    processedFrames++;
}

void FrameStats::frameDisplayed(qint64 recvTsNs, qint64 displayTsNs)
{
    QMutexLocker lock(&mutex);

    displayedFrames++;

    // latence affichage - réception
    double latencyMs =
        double(displayTsNs - recvTsNs) * 1e-6;

    latencySumMs += latencyMs;
    latencyMaxMs = std::max(latencyMaxMs, latencyMs);

    updateWindowLocked(displayTsNs);
}

FrameStats::Snapshot FrameStats::snapshot()
{
    QMutexLocker lock(&mutex);
    return lastSnapshot;
}

void FrameStats::updateWindowLocked(qint64 nowNs)
{
    qint64 elapsed = nowNs - windowStartNs;
    if (elapsed < WINDOW_NS)
        return;

    double seconds = double(elapsed) * 1e-9;

    Snapshot snap;

    snap.inputFps      = inputFrames / seconds;
    snap.processingFps = processedFrames / seconds;
    snap.displayFps    = displayedFrames / seconds;

    droppedFrames =
        (inputFrames > displayedFrames)
            ? (inputFrames - displayedFrames)
            : 0;

    snap.dropRate =
        inputFrames > 0
            ? double(droppedFrames) / double(inputFrames)
            : 0.0;

    snap.avgLatencyMs =
        displayedFrames > 0
            ? latencySumMs / displayedFrames
            : 0.0;

    snap.maxLatencyMs = latencyMaxMs;

    lastSnapshot = snap;

    // reset fenêtre
    inputFrames = 0;
    processedFrames = 0;
    displayedFrames = 0;
    droppedFrames = 0;

    latencySumMs = 0.0;
    latencyMaxMs = 0.0;

    windowStartNs = nowNs;
}

}
