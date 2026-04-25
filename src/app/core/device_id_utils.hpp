// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file device_utils.hpp
 * @brief Utilities for encoding and decoding device identifiers.
 *
 * This module provides helper functions to safely convert device identifiers
 * (QByteArray) into string representations suitable for storage (e.g. settings keys)
 * and back.
 *
 * Encoding is performed using percent-encoding to ensure compatibility with
 * file systems and configuration formats.
 */

#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>

namespace fluvel_app::device
{

/**
 * @brief Encode a device identifier into a safe string representation.
 *
 * Converts a raw device identifier into a percent-encoded UTF-8 string,
 * suitable for use as a key in settings or file paths.
 *
 * @param id Raw device identifier.
 * @return Encoded string representation.
 */
inline QString encodeDeviceId(const QByteArray& id)
{
    return QString::fromUtf8(QUrl::toPercentEncoding(id));
}

/**
 * @brief Decode a previously encoded device identifier.
 *
 * Reconstructs the original device identifier from a percent-encoded string.
 *
 * @param key Encoded string representation.
 * @return Original device identifier.
 */
inline QByteArray decodeDeviceId(const QString& key)
{
    return QUrl::fromPercentEncoding(key.toUtf8()).toUtf8();
}

} // namespace fluvel_app::device
