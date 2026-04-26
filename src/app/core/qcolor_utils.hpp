// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file qcolor_utils.hpp
 * @brief Utilities for color manipulation and icon recoloring.
 *
 * This module provides helper functions to:
 * - adjust color appearance (desaturation, darkening)
 * - generate colorized icons for UI theming
 *
 * These utilities are primarily intended for UI rendering and styling.
 */

#pragma once

#include <QColor>
#include <QIcon>
#include <QtCore/qglobal.h>

class QString;
class QSize;

namespace fluvel_app::qcolor_utils
{

/**
 * @brief Desaturate and darken a color.
 *
 * Applies a saturation and value (brightness) scaling to the input color.
 * Typically used to create disabled, inactive, or secondary UI states.
 *
 * @param original Input color.
 * @param saturationFactor Factor applied to saturation (0.0 → grayscale, 1.0 → unchanged).
 * @param valueFactor Factor applied to value/brightness (0.0 → black, 1.0 → unchanged).
 * @return Transformed color.
 */
QColor desaturateAndDarken(const QColor& original, qreal saturationFactor, qreal valueFactor);

/**
 * @brief Create a colorized version of an icon.
 *
 * Applies a color tint to the given base icon and renders it at the specified size.
 * Useful for dynamic theming (e.g. active/inactive states, accent colors).
 *
 * @param baseIcon Source icon.
 * @param color Tint color.
 * @param size Target icon size.
 * @return Colorized icon.
 */
QIcon colorizeIcon(const QIcon& baseIcon, const QColor& color, const QSize& size);

} // namespace fluvel_app::qcolor_utils
