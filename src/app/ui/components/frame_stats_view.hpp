#ifndef FRAME_STATS_VIEW_HPP
#define FRAME_STATS_VIEW_HPP

#include <QtGlobal>
#include <QMutex>

namespace ofeli_app {

class FrameStatsView
{
public:
    struct Snapshot {
        double inputFps      = 0.0;
        double processingFps = 0.0;
        double displayFps    = 0.0;
        double dropRate      = 0.0;
        double avgLatencyMs  = 0.0;
        double maxLatencyMs  = 0.0;
    };

    FrameStatsView();

    // événements
    void frameReceived(qint64 recvTsNs);
    void frameProcessed();
    void frameDisplayed(qint64 recvTsNs,
                        qint64 displayTsNs);

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

#endif // FRAME_STATS_VIEW_HPP
