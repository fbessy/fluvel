#pragma once

#include <QObject>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QMediaDevices>
#include <QTimer>

#include "video_active_contour_thread.hpp"
#include "frame_stats_view.hpp"
#include "camera_stats.hpp"

namespace ofeli_app {

class CameraController : public QObject
{
    Q_OBJECT

public:
    explicit CameraController(QObject* parent = nullptr);
    ~CameraController();

    void start(const QByteArray& deviceId);
    void stop();
    bool isActive() const;

public slots:

    void onFrameDisplayed(qint64 recvTsNs,
                          qint64 displayTsNs);

signals:
    void frameSizeStr(const QString& str);
    void statsUpdated(const CameraStats& stats);
    void imageAndContourUpdated(const QImage& img,
                                const QVector<QPoint>& l_out,
                                const QVector<QPoint>& l_in,
                                qint64 receiveTs);
private:

    void onFrameResultReady(FrameResult result);
    void onVideoSettingsChanged(const VideoSessionSettings& conf);
    void onVideoDisplaySettingsChanged(const DisplayConfig& displayConfig);

    void onFrameProcessed();
    void updateStats();

    QCamera* camera_ = nullptr;
    QMediaCaptureSession* captureSession_ = nullptr;
    QVideoSink* videoSink_ = nullptr;

    VideoActiveContourThread ac_thread_;

    FrameStatsView frameStats_;
    QTimer* statsTimer_ = nullptr;

    DisplayConfig displayConfig_;
};

} // namespace
