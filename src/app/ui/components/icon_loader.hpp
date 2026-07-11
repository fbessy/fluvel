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
 * @brief Creates a filled circular color icon.
 *
 * The disk is centered within the icon and rendered at a higher
 * resolution before being downscaled to improve visual quality
 * and antialiasing at small sizes.
 *
 * @param color Fill color of the disk.
 * @param size Final icon size in pixels.
 * @param diameter Disk diameter in pixels. A negative value uses
 *        the full icon size.
 *
 * @return A filled circular color icon.
 */
QIcon createDisk(const QColor& color, int size = 13, int diameter = -1);

/**
 * @brief Creates an outlined circular icon.
 *
 * The circle is centered within the icon and rendered using the
 * specified outline color.
 *
 * @param color Outline color of the circle.
 * @param size Final icon size in pixels.
 *
 * @return An outlined circular icon.
 */
QIcon createCircle(const QColor& color, int size = 13);

/**
 * @brief Creates a recording indicator icon.
 *
 * The icon consists of a filled disk surrounded by an outline
 * using the current application palette.
 *
 * @param color Fill color of the recording disk.
 * @param size Final icon size in pixels.
 *
 * @return A recording indicator icon.
 */
QIcon createRecordDisk(const QColor& color, int size = 13);

/**
 * @brief Creates a rounded square color icon.
 *
 * The icon is rendered at a higher resolution and then downscaled
 * to improve visual quality and antialiasing when displayed at
 * small sizes.
 *
 * @param color Fill color of the square.
 * @param size Final icon size in pixels.
 *
 * @return A rounded square color icon.
 */
QIcon createSquare(const QColor& color, int size = 13);

/**
 * @brief Creates an empty transparent icon.
 *
 * This helper can be used to reserve icon space in item views,
 * menus or toolbars when no visible icon is required.
 *
 * @param size Icon size in pixels.
 *
 * @return A transparent icon of the requested size.
 */
QIcon createEmpty(int size = 13);

} // namespace fluvel::il
