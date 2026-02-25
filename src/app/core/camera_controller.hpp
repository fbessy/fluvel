// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "video_active_contour_thread.hpp"
#include "frame_stats_view.hpp"

#include <QCamera>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QObject>
#include <QTimer>
#include <QVideoSink>

namespace ofeli_app
{

class CameraController : public QObject
{
    Q_OBJECT

public:
    explicit CameraController(QObject* parent = nullptr);
    ~CameraController() override;

    void start(const QByteArray& deviceId);
    void stop();
    bool isActive() const;

public slots:

    void onFrameDisplayed(qint64 recvTsNs, qint64 displayTsNs);

signals:
    void frameSizeStr(const QString& str);
    void textStatsUpdated(const QString& textStats);
    void imageAndContourUpdated(const QImage& img, const QVector<QPointF>& l_out,
                                const QVector<QPointF>& l_in, qint64 receiveTs);

private:
    void onFrameResultReady(const FrameResult& result);
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

} // namespace ofeli_app
