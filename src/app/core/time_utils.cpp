// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "time_utils.hpp"

namespace fluvel::time_utils
{

QString formatDuration(qint64 ms)
{
    qint64 sec = ms / 1000;

    qint64 h = sec / 3600;
    qint64 m = (sec % 3600) / 60;
    qint64 s = sec % 60;

    if (h > 0)
        return QString("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));

    return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}

} // namespace fluvel::time_utils