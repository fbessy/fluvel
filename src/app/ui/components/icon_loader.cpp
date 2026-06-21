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

namespace fluvel::il
{

static bool isDarkMode()
{
    return qApp->palette().color(QPalette::Window).lightness() < 128;
}

static QIcon loadSvgWithPalette(const QString& path, IconMode mode)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
        return QIcon(path);

    QString svg = QString::fromUtf8(file.readAll());

    QColor color;

    switch (mode)
    {
        case IconMode::Auto:
        {
            color = qApp->palette().color(QPalette::WindowText);

            if (isDarkMode())
                color = color.lighter(110);

            break;
        }

        case IconMode::Light:
        {
            color = Qt::white;
            break;
        }

        case IconMode::Dark:
        {
            color = Qt::black;
            break;
        }
    }

    static const QRegularExpression kColorRegex(R"(color\s*:\s*#[0-9a-fA-F]+)");
    svg.replace(kColorRegex, "color:" + color.name());

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

const QIcon& appIcon()
{
    static QIcon icon(":/icons/app/fluvel.svg");
    return icon;
}

QIcon desktopAppIcon()
{
    QIcon icon;

    icon.addFile(":/icons/app/fluvel-16.png", QSize(16, 16));

    icon.addFile(":/icons/app/fluvel-22.png", QSize(22, 22));

    icon.addFile(":/icons/app/fluvel-32.png", QSize(32, 32));

    icon.addFile(":/icons/app/fluvel-48.png", QSize(48, 48));

    icon.addFile(":/icons/app/fluvel-64.png", QSize(64, 64));

    icon.addFile(":/icons/app/fluvel-128.png", QSize(128, 128));

    icon.addFile(":/icons/app/fluvel-256.png", QSize(256, 256));

    return icon;
}

QIcon loadIcon(const QString& themeName, const QString& fallback, IconMode mode)
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

    return loadSvgWithPalette(fallback, mode);
}

QIcon loadIcon(QIcon::ThemeIcon iconEnum, const QString& fallback, IconMode mode)
{
#ifndef FLUVEL_FORCE_EMBEDDED_ICONS

    QIcon icon;

    icon = QIcon::fromTheme(iconEnum);
    if (!icon.isNull())
        return icon;

#endif

    return loadSvgWithPalette(fallback, mode);
}

QIcon loadIcon(const QString& svgResourceName, IconMode mode)
{
    return loadSvgWithPalette(svgResourceName, mode);
}

} // namespace fluvel::il
