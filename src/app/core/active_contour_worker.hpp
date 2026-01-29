#ifndef ACTIVE_CONTOUR_WORKER_HPP
#define ACTIVE_CONTOUR_WORKER_HPP

#include <QObject>
#include <QImage>
#include <QMutex>

#include "active_contour.hpp"
#include "algo_stats.hpp"

namespace ofeli_app
{

enum class RunMode
{
    Interactive,  // UI updates
    Converge      // full speed without freeze, no UI updates
};

enum class WorkerState
{
    Uninitialized,   // rien de prêt
    Initializing,    // préparation en cours (CA, buffers, etc.)
    Ready,           // initialisé, prêt
    Suspended,       // pause or mode step
    Running,         // timer actif, algo en cours
    Stopped,         // algo convergé
};

class ActiveContourWorker : public QObject
{
    Q_OBJECT

public:

    ActiveContourWorker();

    void initializeFromImage(const QImage& img);

    void restart();        // reset + start
    void togglePause();    // suspend / resume
    void step();           // one iteration
    void converge();       // converge to the final state and
                           // display only the final result

    void stop();

    AlgoStats currentStats() const;

signals:
    void resultReady(const QImage& img);
    void contourUpdated(const QVector<QPoint>& out,
                        const QVector<QPoint>& in);
    void finished();
    void stateChanged(WorkerState state);

private slots:
    void onTimeout();

private:

    void drawAndEmitResult();
    void emitContourOnly();
    void updateStats();

    void suspend();
    void resume();
    void start();
    void performStep();
    bool stepOnceAlgo();

    void initializeActiveContour();
    void finalizeAndPrepareNextRun();

    void setMode(RunMode mode);
    void setState(WorkerState state);

    WorkerState m_state;
    RunMode m_mode;
    QTimer* m_timer;
    QImage m_workImage;
    std::unique_ptr<ofeli_ip::ActiveContour> ac;

    mutable QMutex m_statsMutex;
    AlgoStats m_currentStats;

    QImage workAlgo;
    QImage initialPhi;

    qint64 timeSlice_ms;
    bool initialShown;
};

}

#endif // ACTIVE_CONTOUR_WORKER_HPP
