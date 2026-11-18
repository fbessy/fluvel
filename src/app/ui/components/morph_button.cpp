// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "morph_button.hpp"

#include <QEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStylePainter>

#include <algorithm>

namespace
{

constexpr qreal kDisabledOpacity = 0.45;

} // namespace

namespace fluvel
{

MorphButton::MorphButton(QWidget* parent)
    : QPushButton(parent)
    , morph_(this)
{
    connect(&morph_, &ShapeMorph::shapeChanged, this, qOverload<>(&QWidget::update));
}

void MorphButton::setShapes(const QList<MorphShape>& shapes)
{
    shapes_ = shapes;
    index_ = 0;

    if (!shapes_.isEmpty())
        morph_.jumpTo(resolvedShape(0));
}

void MorphButton::setShapeIndex(int index)
{
    if (index < 0 || index >= shapes_.size() || index == index_)
        return;

    index_ = index;
    morph_.animateTo(resolvedShape(index));
}

int MorphButton::shapeIndex() const
{
    return index_;
}

MorphShape MorphButton::resolvedShape(int index) const
{
    MorphShape shape = shapes_.at(index);

    if (!shape.color.isValid())
        shape.color = palette().color(QPalette::Active, QPalette::WindowText);

    return shape;
}

QSize MorphButton::sizeHint() const
{
    // No icon or text: size the button as an icon-only button would be.
    QStyleOptionButton option;
    initStyleOption(&option);

    return style()->sizeFromContents(QStyle::CT_PushButton, &option, iconSize(), this);
}

void MorphButton::changeEvent(QEvent* event)
{
    QPushButton::changeEvent(event);

    // Keep the "text color" shapes in sync with theme changes.
    if (event->type() == QEvent::PaletteChange && !shapes_.isEmpty())
        morph_.jumpTo(resolvedShape(index_));
}

void MorphButton::paintEvent(QPaintEvent*)
{
    QStylePainter painter(this);

    QStyleOptionButton option;
    initStyleOption(&option);

    painter.drawControl(QStyle::CE_PushButtonBevel, option);

    if (shapes_.isEmpty())
        return;

    const QRect contents = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);

    const QRect iconRect =
        QStyle::alignedRect(layoutDirection(), Qt::AlignCenter, iconSize(), contents);

    const MorphShape shape = morph_.shape();

    const qreal side = std::min(iconRect.width(), iconRect.height()) * shape.sizeRatio;

    QRectF symbol(0.0, 0.0, side, side);
    symbol.moveCenter(QRectF(iconRect).center());

    const qreal radius = side * shape.radiusRatio;

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(isEnabled() ? 1.0 : kDisabledOpacity);
    painter.setPen(Qt::NoPen);
    painter.setBrush(shape.color);
    painter.drawRoundedRect(symbol, radius, radius);
}

} // namespace fluvel
