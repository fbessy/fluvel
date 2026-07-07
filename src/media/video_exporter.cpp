// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_exporter.hpp"

#include "ffmpeg/ffmpeg_video_exporter.hpp"

#include <QImage>
#include <memory>

namespace fluvel
{

VideoExporter::VideoExporter()
    : exporter_(std::make_unique<FFmpegVideoExporter>())
{
}

VideoExporter::~VideoExporter() = default;

VideoExporter::VideoExporter(VideoExporter&&) noexcept = default;

VideoExporter& VideoExporter::operator=(VideoExporter&&) noexcept = default;

bool VideoExporter::open(const VideoExportSettings& settings)
{
    return exporter_->open(settings);
}

bool VideoExporter::addFrame(const QImage& image)
{
    return exporter_->addFrame(image);
}

bool VideoExporter::close()
{
    return exporter_->close();
}

bool VideoExporter::isOpen() const
{
    return exporter_->isOpen();
}

} // namespace fluvel