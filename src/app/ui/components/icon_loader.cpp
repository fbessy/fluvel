// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "icon_loader.hpp"
#include <QApplication>
#include <QBuffer>
#include <QFile>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QRegularExpression>
#include <QSvgRenderer>

// to check fallbacks icons
// #define FLUVEL_FORCE_EMBEDDED_ICONS

namespace fluvel_app
{

// icon loader namespace for function outside a class
namespace il
{

static bool isDarkMode()
{
    return qApp->palette().color(QPalette::Window).lightness() < 128;
}

static QIcon loadSvgWithPalette(const QString& path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
        return QIcon(path);

    QString svg = QString::fromUtf8(file.readAll());

    QColor color = qApp->palette().color(QPalette::WindowText);

    if (isDarkMode())
        color = color.lighter(110);

    svg.replace(QRegularExpression(R"(color\s*:\s*#[0-9a-fA-F]+)"), "color:" + color.name());

    QByteArray data = svg.toUtf8();

    QIcon icon;

    for (int size : {16, 22, 24, 32})
    {
        QPixmap pixmap(size, size);
        pixmap.fill(Qt::transparent);

        QSvgRenderer renderer(data);

        QPainter painter(&pixmap);
        renderer.render(&painter);

        icon.addPixmap(pixmap);
    }

    return icon;
}

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

    return loadSvgWithPalette(fallback);
}

QIcon loadIcon(QIcon::ThemeIcon iconEnum, const QString& fallback)
{
#ifndef FLUVEL_FORCE_EMBEDDED_ICONS

    QIcon icon;

    icon = QIcon::fromTheme(iconEnum);
    if (!icon.isNull())
        return icon;

#endif

    return loadSvgWithPalette(fallback);
}

QIcon loadIcon(const QString& svgResourceName)
{
    return loadSvgWithPalette(svgResourceName);
}

} // namespace il

} // namespace fluvel_app
