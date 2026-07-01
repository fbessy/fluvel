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

void StyledToolButton::setTransitionEffect(TransitionEffect effect)
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

void StyledToolButton::paintEvent(QPaintEvent*)
{
    QStylePainter painter(this);

    QStyleOptionToolButton option;
    initStyleOption(&option);

    // Draw the button using the current Qt style but
    // suppress the icon since it will be rendered manually.
    option.icon = {};

    painter.drawComplexControl(QStyle::CC_ToolButton, option);

    animatedIcon_.paint(painter, rect(), iconSize(), icon());
}

void StyledToolButton::setAnimatedIcon(const QIcon& icon, TransitionDirection direction)
{
    animatedIcon_.setAnimatedIcon(this->icon(), icon, direction);

    setIcon(icon);
}

AnimatedIcon::TransitionEffect StyledToolButton::transitionEffect() const
{
    return animatedIcon_.transitionEffect();
}

} // namespace fluvel