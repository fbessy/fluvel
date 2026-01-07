#ifndef VIDEO_ACTIVE_CONTOUR_THREAD_H
#define VIDEO_ACTIVE_CONTOUR_THREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QVideoFrame>
#include "frame_stats.hpp"
#include "runtime_settings.hpp"

namespace ofeli_gui {

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

signals:
    void frameProcessed(qint64 receiveTs, qint64 processTs);
    void frameResultReady(const QImage& img, qint64 receiveTs);

protected:
    void run() override;

private:

    RuntimeSettings runtime_settings;

    QImage processFrame(QVideoFrame& frame, qint64& processTs);

    QMutex frameMutex;
    QWaitCondition condition;
    FrameData lastFrameData;
    bool frameAvailable;
    bool running;
    bool configChanged;

    std::unique_ptr<ofeli_ip::RegionColorAc> region_ac;

private slots:
    void reloadSettings();
};

} // namespace ofeli_gui

#endif // VIDEO_ACTIVE_CONTOUR_THREAD_H
