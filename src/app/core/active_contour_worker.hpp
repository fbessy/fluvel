#ifndef ACTIVE_CONTOUR_WORKER_HPP
#define ACTIVE_CONTOUR_WORKER_HPP

#include <QObject>
#include <QImage>
#include <QMutex>

#include "active_contour.hpp"
#include "algo_stats.hpp"

namespace ofeli_app
{

enum class WorkerState
{
    Idle,        // image chargée, pas d'exécution
    Running,     // timer actif, algo en cours
    Stopped,     // algo convergé
    Restarting   // transition atomique
};

class ActiveContourWorker : public QObject
{
    Q_OBJECT

public:

    ActiveContourWorker();

    void setImage(const QImage& img);

    void restart();        // reset + start
    void togglePause();    // suspend / resume
    void step();           // one iteration

    void stop();

    AlgoStats currentStats() const;

signals:
    void resultReady(const QImage& img);
    void contourUpdated(const QVector<QPoint>& out,
                        const QVector<QPoint>& in);
    void finished();

private slots:
    void onTimeout();

private:

    void drawAndEmitResult();
    void emitContourOnly();

    void suspend();
    void resume();
    void start();
    bool stepOnce();

    void updateStats();

    void initializeActiveContour();

    void setState(WorkerState state) { m_state = state; }

    WorkerState m_state;
    QTimer* m_timer;
    QImage m_workImage;
    std::unique_ptr<ofeli_ip::ActiveContour> ac;

    mutable QMutex m_statsMutex;
    AlgoStats m_currentStats;

    QImage workAlgo;
    QImage initialPhi;
};

}

#endif // ACTIVE_CONTOUR_WORKER_HPP
