// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ac_types.hpp"
#include "common_settings.hpp"
#include "frame_data.hpp"
#include "image_view.hpp"
#include "region_color_ac.hpp"
#include "temporal_smoother.hpp"

#include <QVideoFrame>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

namespace fluvel_app
{

class VideoActiveContourThread : public QThread
{
    Q_OBJECT
public:
    VideoActiveContourThread(QObject* parent);

    void submitFrame(const CapturedFrame& capturedFrame);
    void stop();

    void setAlgoConfig(const VideoComputeConfig& config);

signals:
    void frameProcessed(quint64 contourSize);

    void displayFrameReady(fluvel_app::DisplayFrame displayFrame);

    void frameSizeStr(QString str);

protected:
    void run() override;

private:

    QImage convertFrame(QVideoFrame frame) const;
    QImage applyDownscale(const QImage& input, const DownscaleConfig& config) const;
    DisplayFrame processFrame(const QVideoFrame& frame);

    void exportTemporalFilteredImage(const fluvel_ip::ImageView& algoImage,
                                     const VideoComputeConfig& config, DisplayFrame& displayFrame);

    void exportContours(DisplayFrame& displayFrame);

    static constexpr qint64 kTimeSliceMs = 20;

    VideoComputeConfig config_;

    QMutex frameMutex_;
    QWaitCondition condition_;
    CapturedFrame lastCapturedFrame_;
    bool frameAvailable_{false};
    bool running_{true};
    bool configChanged_{false};

    std::unique_ptr<fluvel_ip::RegionColorAc> activeContour_;
    QSize currentSize_ = {0, 0};

    fluvel_ip::TemporalSmoother smoother_;
};

} // namespace fluvel_app
