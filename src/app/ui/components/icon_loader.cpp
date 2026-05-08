// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "icon_loader.hpp"
#include <QApplication>

// to check fallbacks icons
// #define FLUVEL_FORCE_EMBEDDED_ICONS

namespace fluvel_app
{

// icon loader namespace for function outside a class
namespace il
{

QIcon loadIcon(const QString& themeName, const QString& fallback)
{
#ifndef FLUVEL_FORCE_EMBEDDED_ICONS

    QIcon icon;

    icon = QIcon::fromTheme(themeName + "-symbolic");
    if (!icon.isNull())
        return icon;

    icon = QIcon::fromTheme(themeName);
    if (!icon.isNull())
        return icon;

#endif

    return QIcon(fallback);
}

QIcon loadIcon(QIcon::ThemeIcon iconEnum, const QString& fallback)
{
#ifndef FLUVEL_FORCE_EMBEDDED_ICONS

    QIcon icon;

    icon = QIcon::fromTheme(iconEnum);
    if (!icon.isNull())
        return icon;

#endif

    return QIcon(fallback);
}

} // namespace il

} // namespace fluvel_app
