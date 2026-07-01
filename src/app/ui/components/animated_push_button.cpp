// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "animated_push_button.hpp"

#include <QStyleOptionButton>
#include <QStylePainter>

namespace fluvel
{

AnimatedPushButton::AnimatedPushButton(QWidget* parent)
    : QPushButton(parent)
    , animatedIcon_(this)
{
}

void AnimatedPushButton::paintEvent(QPaintEvent*)
{
    QStylePainter painter(this);

    QStyleOptionButton option;
    initStyleOption(&option);

    option.icon = {};

    painter.drawControl(QStyle::CE_PushButton, option);

    animatedIcon_.paint(painter, rect(), iconSize(), icon());
}

void AnimatedPushButton::setAnimatedIcon(const QIcon& icon, TransitionDirection direction)
{
    animatedIcon_.setAnimatedIcon(this->icon(), icon, direction);

    setIcon(icon);
}

AnimatedPushButton::TransitionEffect AnimatedPushButton::transitionEffect() const
{
    return animatedIcon_.transitionEffect();
}

void AnimatedPushButton::setTransitionEffect(TransitionEffect effect)
{
    animatedIcon_.setTransitionEffect(effect);
}

} // namespace fluvel