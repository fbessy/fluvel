// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ac_types.hpp"

#include <QImage>
#include <QVideoFrame>
#include <QtTypes>

namespace fluvel_app
{

struct CapturedFrame
{
    QVideoFrame frame;
    qint64 receiveTimestampNs = 0;
};

struct DisplayFrame
{
    QImage input;
    QImage preprocessed;
    fluvel_ip::ExportedContour outerContour;
    fluvel_ip::ExportedContour innerContour;
    qint64 receiveTimestampNs = 0;
    qint64 processTimestampNs = 0;
};

struct UiFrame
{
    QImage image;

    QVector<QPointF> outerContour;
    QVector<QPointF> innerContour;

    qint64 receiveTimestampNs = 0;
    qint64 processTimestampNs = 0;
};

struct FrameTimestamps
{
    qint64 receiveTimestampNs = 0;
    qint64 processTimestampNs = 0;
    qint64 displayTimestampNs = 0;
};

} // namespace fluvel_app
