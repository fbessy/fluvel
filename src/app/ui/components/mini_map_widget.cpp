// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "mini_map_widget.hpp"

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

namespace fluvel
{

MiniMapWidget::MiniMapWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(120, 120);
}

void MiniMapWidget::setImage(const QImage& image)
{
    if (image.isNull())
    {
        thumbnail_ = QPixmap();
        update();
        return;
    }

    thumbnail_ =
        QPixmap::fromImage(image).scaled(160, 160, Qt::KeepAspectRatio, Qt::FastTransformation);

    update();
}

void MiniMapWidget::setSceneRect(const QRectF& sceneRect)
{
    if (sceneRect_ == sceneRect)
        return;

    sceneRect_ = sceneRect;
    update();
}

void MiniMapWidget::setVisibleRect(const QRectF& visibleRect)
{
    if (visibleRect_ == visibleRect)
        return;

    visibleRect_ = visibleRect;
    update();
}

void MiniMapWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    if (thumbnail_.isNull())
        return;

    QRect imageRect = thumbnail_.rect();
    imageRect.moveCenter(rect().center());

    if (!sceneRect_.isEmpty() && !visibleRect_.isEmpty())
    {
        const double scaleX = imageRect.width() / sceneRect_.width();
        const double scaleY = imageRect.height() / sceneRect_.height();

        QRectF viewportRect(imageRect.left() + visibleRect_.left() * scaleX,
                            imageRect.top() + visibleRect_.top() * scaleY,
                            visibleRect_.width() * scaleX, visibleRect_.height() * scaleY);

        if (viewportRect.contains(imageRect))
            return;

        painter.drawPixmap(imageRect.topLeft(), thumbnail_);

        // ----------------------------------------------------
        // Darken everything outside the visible viewport.
        // ----------------------------------------------------
        QPainterPath outsideArea;
        outsideArea.addRect(QRectF(imageRect));

        QPainterPath visibleArea;
        visibleArea.addRect(viewportRect);

        outsideArea = outsideArea.subtracted(visibleArea);

        painter.fillPath(outsideArea, QColor(0, 0, 0, 110));

        // ----------------------------------------------------
        // Viewport outline.
        // ----------------------------------------------------
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor(230, 230, 230), 2));

        painter.drawRect(viewportRect);

        // ----------------------------------------------------
        // Mini-map border.
        // ----------------------------------------------------
        QColor borderColor = palette().mid().color();
        borderColor.setAlpha(180);
        painter.setPen(QPen(borderColor, 2));

        painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 4, 4);
    }
}

void MiniMapWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return;

    emit centerRequested(scenePositionFromMiniMap(event->position()));
}

void MiniMapWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;

    emit centerRequested(scenePositionFromMiniMap(event->position()));
}

QPointF MiniMapWidget::scenePositionFromMiniMap(const QPointF& position) const
{
    QRect imageRect = thumbnail_.rect();
    imageRect.moveCenter(rect().center());

    const double x = (position.x() - imageRect.left()) / imageRect.width();

    const double y = (position.y() - imageRect.top()) / imageRect.height();

    return QPointF(sceneRect_.left() + x * sceneRect_.width(),
                   sceneRect_.top() + y * sceneRect_.height());
}

} // namespace fluvel