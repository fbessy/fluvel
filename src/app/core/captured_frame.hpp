// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QVideoFrame>
#include <QtTypes>

namespace fluvel_app
{

struct CapturedFrame
{
    QVideoFrame frame;
    qint64 receiveTimestampNs;
};

} // namespace fluvel_app
