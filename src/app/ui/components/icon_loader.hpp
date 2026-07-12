// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QIcon>
#include <QStyle>

/**
 * @file icon_loader.hpp
 *
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

namespace fluvel::il
{

/**
 * @brief Icon color selection mode used for SVG fallback rendering.
 *
 * Auto follows the current application palette.
 * Light forces white icons.
 * Dark forces black icons.
 *
 * @note This mode only affects embedded SVG fallback icons.
 *       Icons loaded from the desktop icon theme are returned
 *       unchanged.
 */
enum class IconMode
{
    Auto,
    Light,
    Dark
};

/**
 * @brief Returns the shared Fluvel application icon.
 *
 * The icon instance is lazily initialized and reused for the
 * lifetime of the application.
 */
const QIcon& appIcon();

/**
 * @brief Returns the desktop application icon.
 *
 * Provides the multi-resolution icon used for desktop window integration.
 */
QIcon desktopAppIcon();

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
 * @param mode Color mode applied to SVG fallback icons.
 *
 * @return Loaded icon.
 */
QIcon loadIcon(const QString& themeName, const QString& fallback, IconMode mode = IconMode::Auto);

/**
 * @brief Loads an icon from the current icon theme using a Qt theme enum.
 *
 * The loading order is:
 * 1. Theme icon
 * 2. Embedded fallback resource
 *
 * @param iconEnum Qt theme icon enum.
 * @param fallback Fallback icon resource path.
 * @param mode Color mode applied to SVG fallback icons.
 *
 * @return Loaded icon.
 */
QIcon loadIcon(QIcon::ThemeIcon iconEnum, const QString& fallback, IconMode mode = IconMode::Auto);

/**
 * @brief Loads an SVG icon from an embedded resource.
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
 * @param mode Color mode applied to SVG fallback icons.
 *
 * @return Loaded icon.
 */
QIcon loadIcon(const QString& svgResourceName, IconMode mode = IconMode::Auto);

/**
 * @brief Creates a filled circular icon.
 *
 * The icon is generated at multiple resolutions and rendered using
 * supersampling to improve antialiasing at small sizes.
 *
 * @param color Fill color of the disk.
 *
 * @return A centered filled circular icon.
 */
QIcon createDisk(const QColor& color);

/**
 * @brief Creates an outlined circular icon.
 *
 * The icon is generated at multiple resolutions and rendered using
 * supersampling to improve antialiasing at small sizes.
 *
 * @param color Outline color of the circle.
 *
 * @return A centered outlined circular icon.
 */
QIcon createCircle(const QColor& color);

/**
 * @brief Creates a filled rounded square icon.
 *
 * The icon is generated at multiple resolutions and rendered using
 * supersampling to improve antialiasing at small sizes.
 *
 * @param color Fill color of the square.
 *
 * @return A centered filled rounded square icon.
 */
QIcon createSquare(const QColor& color);

/**
 * @brief Creates a small filled rounded square icon.
 *
 * The icon is generated at multiple resolutions and rendered using
 * supersampling to improve antialiasing at small sizes.
 *
 * The square uses a reduced visual footprint compared to the regular
 * square icon while remaining centered within the icon.
 *
 * @param color Fill color of the square.
 *
 * @return A centered small filled rounded square icon.
 */
QIcon createSmallSquare(const QColor& color);

/**
 * @brief Creates an empty transparent icon.
 *
 * The icon is generated at multiple resolutions to preserve the same
 * icon layout behavior as visible application icons.
 *
 * It can be used to hide an icon while preserving its allocated space.
 *
 * @return An empty transparent icon.
 */
QIcon createEmpty();

} // namespace fluvel::il
