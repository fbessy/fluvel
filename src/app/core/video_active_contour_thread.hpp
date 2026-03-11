// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ac_types.hpp"
#include "common_settings.hpp"
#include "image_span.hpp"
#include "region_color_ac.hpp"
#include "temporal_smoother.hpp"

#include <QVideoFrame>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

namespace fluvel_app
{

struct FrameResult
{
    QImage input;
    QImage preprocessed;
    fluvel_ip::ExportedContour l_out;
    fluvel_ip::ExportedContour l_in;
    qint64 receiveTs;
    qint64 processTs;
};

struct FrameData
{
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
    void frameProcessed(quint64 contourSize);

    void frameResultReady(fluvel_app::FrameResult result);

    void frameSizeStr(QString str);

protected:
    void run() override;

private:
    VideoComputeConfig config_;

    QImage convertFrame(QVideoFrame frame) const;
    QImage applyDownscale(const QImage& input, const DownscaleConfig& config) const;
    FrameResult processFrame(const QVideoFrame& frame);

    void exportTemporalFilteredImage(const fluvel_ip::ImageSpan& algoImage,
                                     const VideoComputeConfig& config, FrameResult& fr);

    void exportContours(FrameResult& fr);

    QMutex frameMutex_;
    QWaitCondition condition_;
    FrameData lastFrameData_;
    bool frameAvailable_{false};
    bool running_{true};
    bool configChanged_{false};

    std::unique_ptr<fluvel_ip::RegionColorAc> activeContour_;
    QSize currentSize_ = {0, 0};

    fluvel_ip::TemporalSmoother smoother_;
};

} // namespace fluvel_app
