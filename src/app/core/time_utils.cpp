// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "time_utils.hpp"

namespace fluvel::time_utils
{

QString formatDuration(qint64 milliseconds)
{
    const qint64 seconds = milliseconds / 1000;

    const qint64 hours = seconds / 3600;
    const qint64 minutes = (seconds % 3600) / 60;
    const qint64 secs = seconds % 60;

    if (hours > 0)
    {
        return QStringLiteral("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(secs, 2, 10, QLatin1Char('0'));
    }

    return QStringLiteral("%1:%2").arg(minutes).arg(secs, 2, 10, QLatin1Char('0'));
}

QString formatDuration(std::chrono::milliseconds duration)
{
    return formatDuration(duration.count());
}

} // namespace fluvel::time_utils