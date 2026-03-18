// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ac_types.hpp"
#include "common_settings.hpp"
#include "image_view.hpp"
#include "region_color_ac.hpp"
#include "temporal_smoother.hpp"

#include <QVideoFrame>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

namespace fluvel_app
{

struct DisplayFrame
{
    QImage input;
    QImage preprocessed;
    fluvel_ip::ExportedContour outerContour;
    fluvel_ip::ExportedContour innerContour;
    qint64 receiveTimestampNs;
    qint64 processTimestampNs;
};

struct CapturedFrame
{
    QVideoFrame frame;
    qint64 receiveTimestampNs;
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
    void frameProcessed(quint64 contourSize);

    void frameResultReady(fluvel_app::DisplayFrame result);

    void frameSizeStr(QString str);

protected:
    void run() override;

private:

    QImage convertFrame(QVideoFrame frame) const;
    QImage applyDownscale(const QImage& input, const DownscaleConfig& config) const;
    DisplayFrame processFrame(const QVideoFrame& frame);

    void exportTemporalFilteredImage(const fluvel_ip::ImageView& algoImage,
                                     const VideoComputeConfig& config, DisplayFrame& fr);

    void exportContours(DisplayFrame& fr);

    VideoComputeConfig config_;

    QMutex frameMutex_;
    QWaitCondition condition_;
    CapturedFrame lastFrameData_;
    bool frameAvailable_{false};
    bool running_{true};
    bool configChanged_{false};

    std::unique_ptr<fluvel_ip::RegionColorAc> activeContour_;
    QSize currentSize_ = {0, 0};

    fluvel_ip::TemporalSmoother smoother_;
};

} // namespace fluvel_app
