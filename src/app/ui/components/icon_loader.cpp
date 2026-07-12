// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "icon_loader.hpp"
#include <QApplication>
#include <QBuffer>
#include <QFile>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QSvgRenderer>

// to check fallbacks icons
// #define FLUVEL_FORCE_EMBEDDED_ICONS

namespace
{

enum class IconShape
{
    Disk,
    Circle,
    Square,
    SmallSquare
};

constexpr std::array kIconSizes{16, 22, 24, 32, 48, 64};

QIcon createShapeIcon(const QColor& color, IconShape shape)
{
    constexpr int kRenderScale = 4;
    constexpr qreal kPenWidth = 1.5;
    constexpr qreal kCornerRadiusRatio = 0.15;

    QIcon icon;

    for (const int size : kIconSizes)
    {
        const int renderSize = size * kRenderScale;

        QPixmap pixmap(renderSize, renderSize);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        qreal shapeRatio;

        switch (shape)
        {
            case IconShape::Disk:
                shapeRatio = 0.75;
                break;

            case IconShape::Circle:
                shapeRatio = 0.75;
                break;

            case IconShape::Square:
                shapeRatio = 0.75;
                break;

            case IconShape::SmallSquare:
                shapeRatio = 0.45;
                break;
        }

        const qreal shapeSize = renderSize * shapeRatio;
        const qreal offset = (renderSize - shapeSize) * 0.5;

        QRectF rect(offset, offset, shapeSize, shapeSize);

        switch (shape)
        {
            case IconShape::Disk:
                painter.setPen(Qt::NoPen);
                painter.setBrush(color);
                painter.drawEllipse(rect);
                break;

            case IconShape::Circle:
            {
                const qreal penWidth = kPenWidth * kRenderScale;

                rect.adjust(penWidth * 0.5, penWidth * 0.5, -penWidth * 0.5, -penWidth * 0.5);

                QPen pen(color);
                pen.setWidthF(penWidth);

                painter.setPen(pen);
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(rect);
                break;
            }

            case IconShape::Square:
            case IconShape::SmallSquare:
            {
                const qreal radius = shapeSize * kCornerRadiusRatio;

                painter.setPen(Qt::NoPen);
                painter.setBrush(color);
                painter.drawRoundedRect(rect, radius, radius);
                break;
            }
        }

        icon.addPixmap(pixmap.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }

    return icon;
}

} // namespace

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

    svg.replace("<svg ", QString("<svg color=\"%1\" ").arg(color.name()));

    QByteArray data = svg.toUtf8();

    QIcon icon;

    for (const int size : kIconSizes)
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

QIcon createDisk(const QColor& color)
{
    return createShapeIcon(color, IconShape::Disk);
}

QIcon createCircle(const QColor& color)
{
    return createShapeIcon(color, IconShape::Circle);
}

QIcon createSquare(const QColor& color)
{
    return createShapeIcon(color, IconShape::Square);
}

QIcon createSmallSquare(const QColor& color)
{
    return createShapeIcon(color, IconShape::SmallSquare);
}

QIcon createEmpty()
{
    QIcon icon;

    for (const int size : kIconSizes)
    {
        QPixmap pixmap(size, size);
        pixmap.fill(Qt::transparent);

        icon.addPixmap(pixmap);
    }

    return icon;
}

} // namespace fluvel::il
