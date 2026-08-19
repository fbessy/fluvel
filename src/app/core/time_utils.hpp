// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QString>
#include <QtGlobal>

#include <chrono>

namespace fluvel::time_utils
{

/**
 * @brief Formats a duration expressed in milliseconds.
 *
 * The output is in the form "mm:ss" for durations shorter than one hour,
 * or "h:mm:ss" for longer durations.
 *
 * @param milliseconds Duration in milliseconds.
 * @return A formatted duration string.
 */
QString formatDuration(qint64 milliseconds);

/**
 * @brief Formats a duration expressed as std::chrono::milliseconds.
 *
 * This overload delegates to the qint64 overload using @c duration.count().
 *
 * @param duration Duration to format.
 * @return A formatted duration string.
 */
QString formatDuration(std::chrono::milliseconds duration);

} // namespace fluvel::time_utils