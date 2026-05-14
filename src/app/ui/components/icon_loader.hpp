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
 * These functions attempt to load icons from:
 * - the current system icon theme
 * - embedded fallback resources
 *
 * The first available icon is returned.
 *
 * @note Fallback icons are typically Qt resource paths
 *       (e.g. ":/icons/...").
 */
namespace il
{

/**
 * @name Icon loading
 * @brief Load icons from the system theme with embedded fallback resources.
 * @{
 */

/**
 * @brief Loads an icon from the current icon theme using a theme name.
 *
 * The loading order is:
 * 1. Symbolic theme icon
 * 2. Regular theme icon
 * 3. Embedded fallback resource
 *
 * @param themeName Theme icon name.
 * @param fallback Fallback icon resource path.
 *
 * @return Loaded icon.
 */
QIcon loadIcon(const QString& themeName, const QString& fallback);

/**
 * @brief Loads an icon from the current icon theme using a Qt theme enum.
 *
 * The loading order is:
 * 1. Theme icon
 * 2. Embedded fallback resource
 *
 * @param iconEnum Qt theme icon enum.
 * @param fallback Fallback icon resource path.
 *
 * @return Loaded icon.
 */
QIcon loadIcon(QIcon::ThemeIcon iconEnum, const QString& fallback);

/**
 * @brief Load an application icon from an embedded resource.
 *
 * This function loads an SVG icon from the application resources and
 * applies the current Qt palette colors when needed. It is mainly used
 * for fallback symbolic icons in order to improve dark mode integration
 * and cross-platform consistency.
 *
 * Unlike direct QIcon construction, this function ensures that all
 * application icons go through the same loading pipeline.
 *
 * @param resourceName Qt resource path of the icon.
 *
 * @return Loaded icon.
 */
QIcon loadIcon(const QString& svgResourceName);
/** @} */

} // namespace il

} // namespace fluvel_app
