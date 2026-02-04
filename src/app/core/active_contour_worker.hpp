#ifndef ACTIVE_CONTOUR_WORKER_HPP
#define ACTIVE_CONTOUR_WORKER_HPP

#include <QObject>
#include <QImage>
#include <QMutex>

#include "active_contour.hpp"
#include "algo_stats.hpp"
#include "application_settings.hpp"

namespace ofeli_app
{

enum class RunMode
{
    Interactive,  // UI updates
    Converge      // full speed without freeze, no UI updates
};

enum class WorkerState
{
    Uninitialized,
    Initializing,    // prepare active contour
    Ready,
    Suspended,
    Running,         // timer is active
    Finished,        // atomic state because the active contour is reset
                     // to prepare the next run
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

    void finish();

    AlgoStats currentStats() const;

signals:
    void resultReady(const QImage& img);
    void contourUpdated(const QVector<QPoint>& out,
                        const QVector<QPoint>& in);
    void stateChanged(WorkerState state);

private slots:
    void onTimeout();
    void reloadSettings();

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

    ImageSessionSettings config;
};

}

#endif // ACTIVE_CONTOUR_WORKER_HPP
