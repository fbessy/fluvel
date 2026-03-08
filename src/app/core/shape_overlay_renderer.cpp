// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "shape_overlay_renderer.hpp"

#include <QPainter>

namespace fluvel_app
{

ShapeOverlayRenderer::ShapeOverlayRenderer(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);

    setAutoFillBackground(false);
}

void ShapeOverlayRenderer::setShape(const ShapeInfo& shape)
{
    QRect oldRect = lastBoundingBox_;
    shape_ = shape;
    lastBoundingBox_ = shape_.boundingBox;

    // zone minimale à repeindre
    QRect dirty = oldRect.united(lastBoundingBox_).adjusted(-3, -3, 3, 3); // marge pour le pen

    update(dirty);
}

void ShapeOverlayRenderer::paintEvent(QPaintEvent*)
{
    if (!shape_.boundingBox.isValid())
        return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    p.setPen(QPen(Qt::cyan, 2));
    p.setBrush(Qt::NoBrush);

    if (shape_.type == ShapeType::Rectangle)
        p.drawRect(shape_.boundingBox);
    else
        p.drawEllipse(shape_.boundingBox);
}

} // namespace fluvel_app
