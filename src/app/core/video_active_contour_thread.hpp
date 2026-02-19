#ifndef VIDEO_ACTIVE_CONTOUR_THREAD_HPP
#define VIDEO_ACTIVE_CONTOUR_THREAD_HPP

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QVideoFrame>
#include "frame_stats_view.hpp"
#include "temporal_smoother.hpp"
#include "common_settings.hpp"

namespace ofeli_app
{

struct FrameResult
{
    QImage input;
    QImage preprocessed;
    ofeli_ip::ExportedContour l_out;
    ofeli_ip::ExportedContour l_in;
    qint64 receiveTs;
    qint64 processTs;
};

struct FrameData {
    QVideoFrame frame;
    qint64 receiveTs;
};

class VideoActiveContourThread : public QThread
{
    Q_OBJECT
public:
    VideoActiveContourThread(QObject* parent);

    void submitFrame(const QVideoFrame& frame);
    void stop();

    void setAlgoConfig(const VideoComputeConfig& config);

signals:
    void frameProcessed(qint64 receiveTs,
                        qint64 processTs);

    void frameResultReady(FrameResult result);

    void frameSizeStr(QString str);

protected:
    void run() override;

private:

    VideoComputeConfig config_;

    FrameResult processFrame(QVideoFrame& frame);

    QMutex frameMutex;
    QWaitCondition condition;
    FrameData lastFrameData;
    bool frameAvailable;
    bool running;
    bool configChanged;

    std::unique_ptr<ofeli_ip::RegionColorAc> region_ac;
    int currentWidth_  = 0;
    int currentHeight_ = 0;

    ofeli_ip::TemporalSmoother smoother;
};

} // namespace ofeli_app

#endif // VIDEO_ACTIVE_CONTOUR_THREAD_H
