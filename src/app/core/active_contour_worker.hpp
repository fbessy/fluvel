#ifndef ACTIVE_CONTOUR_WORKER_HPP
#define ACTIVE_CONTOUR_WORKER_HPP

#include <QObject>
#include <QImage>
#include "active_contour.hpp"

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
    void start();
    void restart();
    void stop();

signals:
    void resultReady(const QImage& img);

private slots:
    void onTimeout();

private:

    void initializeActiveContour();
    void drawAndEmitResult();

    WorkerState m_state;
    QTimer* m_timer;
    QImage m_workImage;
    std::unique_ptr<ofeli_ip::ActiveContour> ac;
};

}

#endif // ACTIVE_CONTOUR_WORKER_HPP
