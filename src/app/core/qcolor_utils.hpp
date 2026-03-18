// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QColor>
#include <QtTypes>

namespace fluvel_app::qcolor_utils
{

QColor desaturateAndDarken(const QColor& original, qreal saturationFactor, qreal valueFactor);

} // namespace fluvel_app::qcolor_utils
