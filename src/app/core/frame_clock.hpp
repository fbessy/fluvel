// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QElapsedTimer>

namespace ofeli_app
{

class FrameClock
{
public:
    static void init();
    static qint64 nowNs();

private:
    static QElapsedTimer timer;
};

} // namespace ofeli_app
