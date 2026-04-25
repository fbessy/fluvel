// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QIcon>
#include <QStyle>

namespace fluvel_app
{

/**
 * @brief Icon loading utilities with fallback support.
 *
 * These functions attempt to load an icon from multiple sources:
 * - system theme (by name or enum)
 * - Qt standard icons
 * - fallback resource path
 *
 * The first available icon is returned.
 *
 * @note The fallback is typically a resource path (e.g. ":/icons/...").
 */
namespace il
{

/**
 * @name Icon loading
 * @brief Load icons from theme, standard Qt icons, or fallback resources.
 * @{
 */

/**
 * @brief Loads an icon using theme, standard or fallback sources.
 *
 * The loading order is:
 * 1. Theme icon (by name or enum)
 * 2. Qt standard icon (if provided)
 * 3. Fallback path
 *
 * @param themeName Name of the theme icon (optional).
 * @param iconEnum Theme icon enum (optional).
 * @param standardName Qt standard icon (optional).
 * @param fallback Fallback icon path.
 *
 * @return Loaded icon (never null if fallback is valid).
 */

QIcon loadIcon(const QString& themeName, QStyle::StandardPixmap standardName,
               const QString& fallback);
QIcon loadIcon(QIcon::ThemeIcon iconEnum, QStyle::StandardPixmap standardName,
               const QString& fallback);
QIcon loadIcon(const QString& themeName, const QString& fallback);
QIcon loadIcon(QIcon::ThemeIcon iconEnum, const QString& fallback);

/** @} */

} // namespace il

} // namespace fluvel_app
