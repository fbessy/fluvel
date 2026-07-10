// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "video_export_settings.hpp"
#include "video_exporter.hpp"

#include <QImage>

namespace fluvel
{

class IVideoExporter;

/**
 * @brief Records rendered frames into a video.
 *
 * This class owns a video exporter and provides a simple recording API.
 * The caller controls when recording starts and stops, and submits the
 * rendered frames to record.
 */
class VideoRecorder
{
public:
    VideoRecorder();
    ~VideoRecorder();

    VideoRecorder(const VideoRecorder&) = delete;
    VideoRecorder& operator=(const VideoRecorder&) = delete;

    VideoRecorder(VideoRecorder&&) = delete;
    VideoRecorder& operator=(VideoRecorder&&) = delete;

    [[nodiscard]]
    bool start(const VideoExportSettings& settings);

    [[nodiscard]]
    bool stop();

    [[nodiscard]]
    bool isRecording() const;

    [[nodiscard]]
    bool addFrame(const QImage& image);

private:
    VideoExporter exporter_;

    bool recording_{false};
};

} // namespace fluvel