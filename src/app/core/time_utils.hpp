// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QString>
#include <QtGlobal>

namespace fluvel::time_utils
{

/**
 * @brief Formats a duration in milliseconds into a human-readable string.
 *
 * Converts a duration expressed in milliseconds into a formatted string.
 * The output is typically in the form "hh:mm:ss" or "mm:ss" depending on
 * the duration length.
 *
 * @param ms Duration in milliseconds.
 * @return A formatted string representing the duration.
 */
QString formatDuration(qint64 ms);
}