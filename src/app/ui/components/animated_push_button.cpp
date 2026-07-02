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

void AnimatedPushButton::setAnimatedIcon(const QIcon& icon, TransitionDirection direction)
{
    animatedIcon_.setAnimatedIcon(this->icon(), icon, direction);

    setIcon(icon);
}

void AnimatedPushButton::paintEvent(QPaintEvent*)
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

    QStyleOptionButton option;
    initStyleOption(&option);

    option.icon = {};

    painter.drawControl(QStyle::CE_PushButton, option);

    animatedIcon_.paint(painter, rect(), iconSize(), icon());

    if (!qFuzzyCompare(s, 1.0))
        painter.restore();
}

AnimatedPushButton::TransitionEffect AnimatedPushButton::transitionEffect() const
{
    return animatedIcon_.transitionEffect();
}

void AnimatedPushButton::setTransitionEffect(AnimatedPushButton::TransitionEffect effect)
{
    animatedIcon_.setTransitionEffect(effect);
}

void AnimatedPushButton::mousePressEvent(QMouseEvent* event)
{
    if (clickAnimation_ == ClickAnimation::Scale)
        scaleAnimation_.start();

    QPushButton::mousePressEvent(event);
}

ClickAnimation AnimatedPushButton::clickAnimation() const
{
    return clickAnimation_;
}

void AnimatedPushButton::setClickAnimation(ClickAnimation animation)
{
    clickAnimation_ = animation;
}

} // namespace fluvel