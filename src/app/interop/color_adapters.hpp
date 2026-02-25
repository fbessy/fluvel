// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"
#include <QColor>

inline QColor toQColor(const ofeli_ip::Rgb_uc& c)
{
    return QColor(static_cast<int>(c.red), static_cast<int>(c.green), static_cast<int>(c.blue));
}

inline ofeli_ip::Rgb_uc toRgb_uc(const QColor& c)
{
    return ofeli_ip::Rgb_uc{static_cast<unsigned char>(c.red()),
                            static_cast<unsigned char>(c.green()),
                            static_cast<unsigned char>(c.blue())};
}

inline ofeli_ip::Rgb_uc toRgb_uc(QRgb pixel)
{
    return {static_cast<unsigned char>(qRed(pixel)), static_cast<unsigned char>(qGreen(pixel)),
            static_cast<unsigned char>(qBlue(pixel))};
}
