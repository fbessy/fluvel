// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "contour_types.hpp"

#include <QImage>
#include <QVideoFrame>
#include <QtTypes>

namespace fluvel_app
{

struct CapturedFrame
{
    QVideoFrame frame;
    int64_t receiveTimestampNs{0};
};

struct DisplayFrame
{
    QImage image;
    fluvel_ip::ExportedContour outerContour;
    fluvel_ip::ExportedContour innerContour;
    int64_t receiveTimestampNs{0};
    int64_t processTimestampNs{0};
};

struct UiFrame
{
    QImage image;

    QVector<QPointF> outerContour;
    QVector<QPointF> innerContour;

    int64_t receiveTimestampNs{0};
    int64_t processTimestampNs{0};
};

struct FrameTimestamps
{
    int64_t receiveTimestampNs{0};
    int64_t processTimestampNs{0};
    int64_t displayTimestampNs{0};
};

} // namespace fluvel_app
