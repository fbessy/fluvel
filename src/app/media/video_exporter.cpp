// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_exporter.hpp"

#ifdef FLUVEL_USE_FFMPEG

#include "ffmpeg_video_exporter.hpp"

#endif

#include <QImage>
#include <memory>

namespace fluvel
{

VideoExporter::VideoExporter()
{
#ifdef FLUVEL_USE_FFMPEG
    exporter_ = std::make_unique<FFmpegVideoExporter>();
#endif
}

VideoExporter::~VideoExporter() = default;

VideoExporter::VideoExporter(VideoExporter&&) noexcept = default;

VideoExporter& VideoExporter::operator=(VideoExporter&&) noexcept = default;

bool VideoExporter::open(const VideoExportSettings& settings)
{
    return exporter_ && exporter_->open(settings);
}

bool VideoExporter::addFrame(const VideoFrame& frame)
{
    return exporter_ && exporter_->addFrame(frame);
}

bool VideoExporter::close()
{
    return exporter_ && exporter_->close();
}

bool VideoExporter::isRecording() const
{
    return exporter_ && exporter_->isRecording();
}

} // namespace fluvel