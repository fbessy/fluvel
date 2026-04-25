// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"
#include <QColor>

/**
 * @brief Convert a Fluvel RGB color to a Qt QColor.
 *
 * Performs a direct component-wise conversion from 8-bit RGB to QColor.
 *
 * @param c Input Fluvel RGB color.
 * @return Equivalent QColor instance.
 */
inline QColor toQColor(const fluvel_ip::Rgb_uc& c)
{
    return QColor(static_cast<int>(c.red), static_cast<int>(c.green), static_cast<int>(c.blue));
}

/**
 * @brief Convert a Qt QColor to a Fluvel RGB color.
 *
 * Extracts RGB components from QColor and converts them to 8-bit unsigned values.
 *
 * @param c Input QColor.
 * @return Equivalent Fluvel RGB color.
 */
inline fluvel_ip::Rgb_uc toRgb_uc(const QColor& c)
{
    return fluvel_ip::Rgb_uc{static_cast<unsigned char>(c.red()),
                             static_cast<unsigned char>(c.green()),
                             static_cast<unsigned char>(c.blue())};
}

/**
 * @brief Convert a Qt QRgb pixel to a Fluvel RGB color.
 *
 * Extracts RGB components using Qt helper functions (qRed, qGreen, qBlue).
 *
 * @param pixel Input QRgb pixel value.
 * @return Equivalent Fluvel RGB color.
 */
inline fluvel_ip::Rgb_uc toRgb_uc(QRgb pixel)
{
    return {static_cast<unsigned char>(qRed(pixel)), static_cast<unsigned char>(qGreen(pixel)),
            static_cast<unsigned char>(qBlue(pixel))};
}
