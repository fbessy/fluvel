#ifndef ACTIVE_CONTOUR_WORKER_HPP
#define ACTIVE_CONTOUR_WORKER_HPP

#include <QObject>
#include <QImage>
#include <QMutex>

#include "active_contour.hpp"
#include "algo_stats.hpp"
#include "common_settings.hpp"

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

    void initializeFromInput(const QImage& input,
                             const ImageSessionSettings& config);

    void restart();        // reset + start
    void togglePause();    // suspend / resume
    void step();           // one iteration
    void converge();       // converge to the final state and
                           // display only the final result

    void finish();

    AlgoStats currentStats() const;

    void setAlgoConfig(const ImageSessionSettings& config);

signals:
    void processedImageReady(const QImage& img);
    void contourUpdated(const ofeli_ip::ExportedContour& l_out,
                        const ofeli_ip::ExportedContour& l_in);
    void stateChanged(ofeli_app::WorkerState state);

private slots:
    void onTimeout();

private:

    void emitContour();
    void updateStats();

    void suspend();
    void resume();
    void start();
    void performStep();
    bool stepOnceAlgo();

    void applyPreprocess();
    void applyDownscale();
    void applyProcessing();
    void initializeActiveContour();
    void finalizeAndPrepareNextRun();

    void setMode(RunMode mode);
    void setState(WorkerState state);

    WorkerState state_;
    RunMode mode_;
    QTimer* timer_;
    std::unique_ptr<ofeli_ip::ActiveContour> ac_;

    mutable QMutex statsMutex_;
    AlgoStats currentStats_;

    QImage inputImage_;
    QImage downscaledImage_;
    QImage processedImage_;
    QImage scaledPhi_;

    qint64 timeSlice_ms_;
    bool initialShown_;

    ImageSessionSettings config_;
};

}

#endif // ACTIVE_CONTOUR_WORKER_HPP
