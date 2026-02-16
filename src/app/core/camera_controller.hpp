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

signals:
    void imageReady(const QImage& img);
    void frameSizeStr(const QString& str);
    void statsUpdated(const CameraStats& stats);

private slots:
    void onFrameProcessed();
    void onFrameResultReady(const QImage& img,
                            qint64 recvTs);
    void updateStats();

private:

    void onCamSettingsChanged(const CameraSessionSettings& conf);
    void onCamDisplaySettingsChanged(const DisplayConfig& disp_config);

    QCamera* camera_ = nullptr;
    QMediaCaptureSession* captureSession_ = nullptr;
    QVideoSink* videoSink_ = nullptr;

    VideoActiveContourThread ac_thread_;

    FrameStatsView frameStats_;
    QTimer* statsTimer_ = nullptr;
};

} // namespace
