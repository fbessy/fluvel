#ifndef FRAME_STATS_HPP
#define FRAME_STATS_HPP

#include <QtGlobal>
#include <QMutex>

namespace ofeli_gui {

class FrameStats
{
public:
    struct Snapshot {
        float inputFps = 0.f;
        float processingFps = 0.f;
        float displayFps = 0.f;
        float dropRate = 0.f;
        float avgLatencyMs = 0.f;
        float maxLatencyMs = 0.f;
    };

    FrameStats();

    // événements
    void frameReceived(qint64 recvTsNs);
    void frameProcessed();
    void frameDisplayed(qint64 recvTsNs, qint64 displayTsNs);

    // lecture stable (fenêtre ~1s)
    Snapshot snapshot();

    void reset();

private:
    void updateWindowLocked(qint64 nowNs);

private:
    QMutex mutex;

    // compteurs fenêtre courante
    quint64 inputFrames;
    quint64 processedFrames;
    quint64 displayedFrames;
    quint64 droppedFrames;

    // latence fenêtre
    double latencySumMs;
    double latencyMaxMs;

    // timestamps
    qint64 windowStartNs;

    // snapshot précédent (pour affichage stable)
    Snapshot lastSnapshot;
};

}

#endif // FRAME_STATS_HPP
