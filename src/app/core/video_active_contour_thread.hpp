// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour.hpp"
#include "common_settings.hpp"
#include "frame_data.hpp"
#include "image_view.hpp"
#include "temporal_smoother.hpp"

#include <QVideoFrame>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include <atomic>
#include <chrono>

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
    void setDisplayMode(ImageDisplayMode mode);

signals:
    void frameProcessed(quint64 contourSize);
    void displayFrameReady(fluvel_app::DisplayFrame displayFrame);

protected:
    void run() override;

private:

    QImage convertFrame(QVideoFrame frame) const;
    QImage applyDownscale(const QImage& input, const DownscaleConfig& config) const;
    DisplayFrame processFrame(const QVideoFrame& inputFrame);

    void exportTemporalFilteredImage(const fluvel_ip::ImageView& algoImage,
                                     DisplayFrame& displayFrame);

    void exportContours(DisplayFrame& displayFrame);

    static constexpr std::chrono::milliseconds kTimeSliceMs{20};

    VideoComputeConfig config_;
    ImageDisplayMode displayMode_;

    QMutex frameMutex_;
    QMutex configMutex_;
    QWaitCondition condition_;
    CapturedFrame lastCapturedFrame_;
    bool frameAvailable_{false};
    std::atomic<bool> running_{false};
    bool configChanged_{false};
    bool displayModeChanged_{false};

    std::unique_ptr<fluvel_ip::ActiveContour> activeContour_;
    QSize currentSize_ = {0, 0};

    fluvel_ip::TemporalSmoother smoother_;

    CapturedFrame buffers_[2];
    std::atomic<int> writeIndex_{0};
    std::atomic<bool> hasNewFrame_{false};
};

} // namespace fluvel_app
