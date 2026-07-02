// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "styled_tool_button.hpp"
#include "qcolor_utils.hpp"
#include "ui_theme.hpp"

#include <QStyleOptionToolButton>
#include <QStylePainter>

namespace fluvel
{

StyledToolButton::StyledToolButton(QWidget* parent, ui::Appearance appearance)
    : QToolButton(parent)
    , appearance_(appearance)
    , animatedIcon_(this)
{
    updateStyle();

    setCursor(Qt::PointingHandCursor);

    const auto bSize = ui::kButtonSize;
    const auto iSize = ui::kButtonIconSize;

    setFixedSize(bSize, bSize);
    setIconSize(QSize(iSize, iSize));
}

void StyledToolButton::setAnimatedIcon(const QIcon& icon, TransitionDirection direction)
{
    animatedIcon_.setAnimatedIcon(this->icon(), icon, direction);

    setIcon(icon);
}

void StyledToolButton::paintEvent(QPaintEvent*)
{
    QStylePainter painter(this);

    const qreal s = scaleAnimation_.scale();

    if (!qFuzzyCompare(s, 1.0))
    {
        painter.save();

        const QPointF center(rect().width() / 2.0, rect().height() / 2.0);

        painter.translate(center);
        painter.scale(s, s);
        painter.translate(-center);
    }

    QStyleOptionToolButton option;
    initStyleOption(&option);

    // Draw the button using the current Qt style but
    // suppress the icon since it will be rendered manually.
    option.icon = {};

    painter.drawComplexControl(QStyle::CC_ToolButton, option);

    animatedIcon_.paint(painter, rect(), iconSize(), icon());

    if (!qFuzzyCompare(s, 1.0))
        painter.restore();
}

void StyledToolButton::mousePressEvent(QMouseEvent* event)
{
    if (clickAnimation_ == ClickAnimation::Scale)
        scaleAnimation_.start();

    QToolButton::mousePressEvent(event);
}

ui::Appearance StyledToolButton::appearance() const
{
    return appearance_;
}

void StyledToolButton::setAppearance(ui::Appearance appearance)
{
    if (appearance_ == appearance)
        return;

    appearance_ = appearance;

    updateStyle();
}

ClickAnimation StyledToolButton::clickAnimation() const
{
    return clickAnimation_;
}

void StyledToolButton::setClickAnimation(ClickAnimation animation)
{
    clickAnimation_ = animation;
}

StyledToolButton::TransitionEffect StyledToolButton::transitionEffect() const
{
    return animatedIcon_.transitionEffect();
}

void StyledToolButton::setTransitionEffect(StyledToolButton::TransitionEffect effect)
{
    animatedIcon_.setTransitionEffect(effect);
}

void StyledToolButton::updateStyle()
{
    if (appearance_ == ui::Appearance::Native)
    {
        setStyleSheet({});
        return;
    }

    setStyleSheet(QString(R"(

QToolButton
{
    background: %1;
    border: 1px solid %2;
    border-radius: %3px;
}

QToolButton:hover
{
    background: %4;
}

QToolButton:pressed
{
    background: %5;
}

QToolButton:disabled
{
    background: %6;
    border: 1px solid %7;
}

QToolButton:checked
{
    background: %8;
    border: 1px solid %8;
}

QToolButton:checked:hover
{
    background: %9;
    border: 1px solid %9;
}

)")
                      .arg(qcolor_utils::rgba(ui::kControlBackground))
                      .arg(qcolor_utils::rgba(ui::kControlBorder))
                      .arg(ui::kControlRadius)
                      .arg(qcolor_utils::rgba(ui::kControlHover))
                      .arg(qcolor_utils::rgba(ui::kControlPressed))
                      .arg(qcolor_utils::rgba(ui::kControlDisabled))
                      .arg(qcolor_utils::rgba(ui::kControlDisabledBorder))
                      .arg(qcolor_utils::rgba(ui::kAccentColor))
                      .arg(qcolor_utils::rgba(ui::kAccentHoverColor)));
}

} // namespace fluvel