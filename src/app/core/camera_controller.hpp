// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "common_settings.hpp"
#include "frame_stats_view.hpp"
#include "video_active_contour_thread.hpp"

#include <QByteArray>
#include <QCamera>
#include <QObject>
#include <QtTypes>

class QMediaCaptureSession;
class QVideoSink;
class QTimer;

namespace fluvel_app
{

class CameraController : public QObject
{
    Q_OBJECT

public:
    explicit CameraController(const VideoSessionSettings& session, QObject* parent = nullptr);
    ~CameraController() override;

    void start(const QByteArray& deviceId);
    void stop();
    bool isActive() const;

    void onVideoSettingsChanged(const VideoSessionSettings& session);
    void onVideoDisplaySettingsChanged(const DisplayConfig& display);

    void onFrameDisplayed(qint64 recvTsNs, qint64 displayTsNs);

signals:
    void cameraStarted(const QByteArray& deviceId);
    void cameraStopped(const QByteArray& deviceId);
    void cameraError(const QByteArray& deviceId, QCamera::Error error, const QString& errorString);

    void frameSizeStr(const QString& str);
    void textStatsUpdated(const QString& textStats);
    void imageAndContourUpdated(const QImage& img, const QVector<QPointF>& l_out,
                                const QVector<QPointF>& l_in, qint64 receiveTs);

private:
    void onVideoInputsChanged();
    void onCameraActiveChanged(bool active);
    void onCameraError(QCamera::Error error, const QString& errorString);
    void onVideoFrame(const QVideoFrame& frame);

    void onFrameProcessed(quint64 contourSize);
    void onFrameResultReady(const FrameResult& result);
    void updateDiagnostics();

    QCamera* camera_ = nullptr;
    QMediaCaptureSession* captureSession_ = nullptr;
    QVideoSink* videoSink_ = nullptr;

    VideoActiveContourThread activeContourThread_;

    FrameStatsView frameStats_;
    QTimer* statsTimer_ = nullptr;

    DisplayConfig displayConfig_;

    bool streamStarted_{false};
    QByteArray activeDeviceId_;
};

} // namespace fluvel_app
