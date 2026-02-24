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
    QMutex mutex_;

    // compteurs fenêtre courante
    quint64 inputFrames_;
    quint64 processedFrames_;
    quint64 displayedFrames_;
    quint64 droppedFrames_;

    // latence fenêtre
    double latencySumMs_;
    double latencyMaxMs_;

    // timestamps
    qint64 windowStartNs_;

    // snapshot précédent (pour affichage stable)
    Snapshot lastSnapshot_;
};

}

#endif // FRAME_STATS_VIEW_HPP
