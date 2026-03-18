// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "qcolor_utils.hpp"

namespace fluvel_app::qcolor_utils
{

QColor desaturateAndDarken(const QColor& original, qreal saturationFactor, qreal valueFactor)
{
    // Clamp des facteurs pour éviter les aberrations
    saturationFactor = std::clamp(saturationFactor, 0.0, 1.0);
    valueFactor = std::clamp(valueFactor, 0.0, 1.0);

    QColor hsv = original.toHsv();

    int h = hsv.hue();        // peut être -1 si gris
    int s = hsv.saturation(); // 0 → gris pur
    int v = hsv.value();
    int a = hsv.alpha();

    // Si gris pur → on ne touche qu'à la luminosité
    if (s == 0)
    {
        v = static_cast<int>(v * valueFactor);
        return QColor::fromHsv(0, 0, v, a);
    }

    // Couleur normale
    s = static_cast<int>(s * saturationFactor);
    v = static_cast<int>(v * valueFactor);

    return QColor::fromHsv(h, s, v, a);
}

} // namespace qcolor_utils
