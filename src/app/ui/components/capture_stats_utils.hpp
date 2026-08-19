// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#ifdef FLUVEL_USE_FFMPEG

#include "recording_types.hpp"

#include <QString>

namespace fluvel::capture_utils
{

struct RecordingStatus
{
    RecorderState state{RecorderState::Stopped};
    QString text;
};

/**
 * @brief Formats the current recording status for display.
 *
 * @param state Current recorder state.
 * @param stats Current recorder statistics.
 * @return Recording status containing the recorder state and formatted text.
 */
RecordingStatus formatRecordingStatus(RecorderState state, const RecorderStats& stats);

} // namespace fluvel::capture_utils

#endif