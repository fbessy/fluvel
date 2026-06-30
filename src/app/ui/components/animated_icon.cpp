// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "animated_icon.hpp"

#include <QPainter>
#include <QPropertyAnimation>
#include <QWidget>
#include <QtMath>

namespace
{

constexpr int kFlipDurationMs = 220;
constexpr qreal kFlipAngle = 80.0;

} // namespace

namespace fluvel
{

AnimatedIcon::AnimatedIcon(QWidget* owner)
    : QObject(owner)
    , owner_(owner)
{
}

void AnimatedIcon::paint(QPainter& painter, const QRect& rect, const QSize& iconSize,
                         const QIcon& defaultIcon) const
{
    const QPointF center(rect.width() / 2.0, rect.height() / 2.0);

    auto drawIcon = [&](const QIcon& icon, qreal opacity, qreal angle)
    {
        if (icon.isNull() || opacity <= 0.0)
            return;

        const QPixmap pixmap = icon.pixmap(iconSize);

        const qreal sx = std::abs(std::cos(qDegreesToRadians(angle)));

        painter.save();

        painter.translate(center);

        painter.rotate(angle);

        painter.scale(sx, 1.0);

        painter.setOpacity(opacity);

        painter.translate(-pixmap.width() / 2.0, -pixmap.height() / 2.0);

        painter.drawPixmap(QPointF(), pixmap);

        painter.restore();
    };

    if (currentIcon_.isNull())
    {
        drawIcon(defaultIcon, 1.0, 0.0);
        return;
    }

    const qreal p = transitionProgress_;

    const qreal angle = transitionDirection_ * kFlipAngle * std::sin(p * M_PI);

    drawIcon(currentIcon_, 1.0 - p, angle);

    drawIcon(nextIcon_, p, -angle);
}

void AnimatedIcon::setAnimatedIcon(const QIcon& current, const QIcon& next, FlipDirection direction)
{
    if (current.cacheKey() == next.cacheKey())
        return;

    currentIcon_ = current;
    nextIcon_ = next;

    updateTransitionDirection(direction);

    if (animation_)
    {
        animation_->stop();
        transitionProgress_ = 0.0;
    }

    animation_ = new QPropertyAnimation(this, "transitionProgress");

    animation_->setDuration(kFlipDurationMs);
    animation_->setStartValue(0.0);
    animation_->setEndValue(1.0);

    animation_->setEasingCurve(QEasingCurve::InOutSine);

    connect(animation_, &QPropertyAnimation::finished, this,
            [this]()
            {
                currentIcon_ = {};
                nextIcon_ = {};

                transitionProgress_ = 0.0;

                animation_ = nullptr;

                if (owner_)
                    owner_->update();
            });

    animation_->start(QAbstractAnimation::DeleteWhenStopped);
}

bool AnimatedIcon::isAnimating() const
{
    return animation_;
}

qreal AnimatedIcon::transitionProgress() const
{
    return transitionProgress_;
}

void AnimatedIcon::setTransitionProgress(qreal progress)
{
    if (qFuzzyCompare(progress, transitionProgress_))
        return;

    transitionProgress_ = progress;

    if (owner_)
        owner_->update();
}

void AnimatedIcon::updateTransitionDirection(FlipDirection direction)
{
    switch (direction)
    {
        case FlipDirection::Left:
            transitionDirection_ = -1.0;
            break;

        case FlipDirection::Right:
            transitionDirection_ = 1.0;
            break;

        case FlipDirection::Auto:
            transitionDirection_ *= -1.0;
            break;
    }
}

} // namespace fluvel