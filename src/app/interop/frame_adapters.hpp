// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_span.hpp"
#include <QVideoFrame>

struct VideoFrame
{
    QVideoFrame frame;
    std::vector<unsigned char> buffer;
    fluvel_ip::ImageSpan span;

    int width{0};
    int height{0};
    fluvel_ip::ImageFormat format{fluvel_ip::ImageFormat::Gray8};
};

std::optional<VideoFrame> video_frame_from_qvideoframe(QVideoFrame frame);
