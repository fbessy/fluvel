// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "capture_stats_utils.hpp"

#include "time_utils.hpp"

#include <QObject>

namespace fluvel::capture_utils
{

RecordingStatus formatRecordingStatus(RecorderState state, const RecorderStats& stats)
{
    RecordingStatus status;
    status.state = state;

    switch (state)
    {
        case RecorderState::Recording:
        {
            status.text =
                QObject::tr("Recorded: %1").arg(time_utils::formatDuration(stats.recordedDuration));

            if (stats.estimatedMaxRecordedDuration)
            {
                status.text += QString(" / ~%1").arg(
                    time_utils::formatDuration(*stats.estimatedMaxRecordedDuration));
            }

            break;
        }

        case RecorderState::Draining:
        {
            status.text = QObject::tr("Finalizing... Writing remaining %1")
                              .arg(time_utils::formatDuration(stats.retainedDuration));
            break;
        }

        case RecorderState::Stopped:
            return status;
    }

    const double queuedMemoryMiB = static_cast<double>(stats.queuedMemoryBytes) / (1024.0 * 1024.0);

    const double queuedDiskMiB = static_cast<double>(stats.queuedDiskBytes) / (1024.0 * 1024.0);

    status.text += QObject::tr(" · %1 MiB RAM").arg(queuedMemoryMiB, 0, 'f', 0);

    if (stats.queuedDiskBytes > 0)
    {
        status.text += QObject::tr(" + %1 MiB temporary").arg(queuedDiskMiB, 0, 'f', 0);
    }

    if (stats.discardedFrames > 0)
    {
        double discardedPercent = 0.0;

        if (stats.submittedFrames != 0)
        {
            discardedPercent = 100.0 * stats.discardedFrames / stats.submittedFrames;
        }

        status.text += QObject::tr(" · Discarded frames: %1% (%2)")
                           .arg(discardedPercent, 0, 'f', 1)
                           .arg(stats.discardedFrames);
    }

    return status;
}

} // namespace fluvel::capture_utils