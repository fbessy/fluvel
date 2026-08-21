// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "animated_icon.hpp"

#include <QAbstractButton>
#include <QEasingCurve>
#include <QPainter>
#include <QPropertyAnimation>
#include <QWidget>
#include <QtMath>
#include <QtNumeric>

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
                         const QIcon& defaultIcon)
{
    switch (transitionEffect_)
    {
        case TransitionEffect::Flip:
            paintFlip(painter, rect, iconSize, defaultIcon);
            break;

        case TransitionEffect::Slide:
            paintSlide(painter, rect, iconSize, defaultIcon);
            break;
    }
}

void AnimatedIcon::paintFlip(QPainter& painter, const QRect& rect, const QSize& iconSize,
                             const QIcon& defaultIcon)
{
    const QPointF center = rectCenter(rect);

    if (currentIcon_.isNull())
    {
        drawIcon(painter, center, iconSize, defaultIcon, 1.0, 0.0, 0.0, 1.0);
        return;
    }

    const qreal p = transitionProgress_;

    const qreal angle = transitionDirection_ * kFlipAngle * std::sin(p * M_PI);

    drawIcon(painter, center, iconSize, currentIcon_, 1.0 - p, angle, 0.0, 1.0);

    drawIcon(painter, center, iconSize, nextIcon_, p, -angle, 0.0, 1.0);
}

void AnimatedIcon::paintSlide(QPainter& painter, const QRect& rect, const QSize& iconSize,
                              const QIcon& defaultIcon)
{
    const QPointF center = rectCenter(rect);

    if (currentIcon_.isNull())
    {
        drawIcon(painter, center, iconSize, defaultIcon, 1.0, 0.0, 0.0, 1.0);
        return;
    }

    const qreal p = transitionProgress_;

    static const QEasingCurve easing(QEasingCurve::InOutCubic);

    const qreal t = easing.valueForProgress(p);

    constexpr qreal kSlideDistance = 10.0;
    constexpr qreal kScaleMin = 0.90;

    // Outgoing icon
    drawIcon(painter, center, iconSize, currentIcon_, 1.0 - t, 0.0,
             transitionDirection_ * kSlideDistance * t, 1.0 - (1.0 - kScaleMin) * t);

    // Incoming icon
    drawIcon(painter, center, iconSize, nextIcon_, t, 0.0,
             -transitionDirection_ * kSlideDistance * (1.0 - t), kScaleMin + (1.0 - kScaleMin) * t);
}

void AnimatedIcon::drawIcon(QPainter& painter, const QPointF& center, const QSize& iconSize,
                            const QIcon& icon, qreal opacity, qreal angle, qreal offsetX,
                            qreal scale) const
{
    if (icon.isNull() || opacity <= 0.0)
        return;

    QIcon::Mode mode = QIcon::Normal;
    QIcon::State state = QIcon::Off;
    qreal finalOpacity = opacity;

    if (owner_)
    {
        if (!owner_->isEnabled())
        {
            mode = QIcon::Disabled;
            finalOpacity *= 0.45;
        }

        if (const auto* button = qobject_cast<const QAbstractButton*>(owner_))
        {
            if (button->isChecked())
                state = QIcon::On;
        }
    }

    const QPixmap pixmap = icon.pixmap(iconSize, mode, state);

    painter.save();

    painter.translate(center);

    if (!qFuzzyIsNull(angle))
    {
        painter.rotate(angle);

        const qreal sx = std::abs(std::cos(qDegreesToRadians(angle)));

        painter.scale(sx, 1.0);
    }

    painter.scale(scale, scale);

    painter.setOpacity(finalOpacity);

    painter.translate(offsetX, 0.0);

    const qreal dpr = pixmap.devicePixelRatio();
    const QSizeF logicalSize = dpr > 0.0 ? QSizeF(pixmap.size()) / dpr : QSizeF(pixmap.size());

    painter.translate(-logicalSize.width() / 2.0, -logicalSize.height() / 2.0);

    painter.drawPixmap(QPointF(), pixmap);

    painter.restore();
}

void AnimatedIcon::setAnimatedIcon(const QIcon& current, const QIcon& next,
                                   TransitionDirection direction)
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

void AnimatedIcon::updateTransitionDirection(TransitionDirection direction)
{
    switch (direction)
    {
        case TransitionDirection::Left:
            transitionDirection_ = -1.0;
            break;

        case TransitionDirection::Right:
            transitionDirection_ = 1.0;
            break;

        case TransitionDirection::Auto:
            transitionDirection_ *= -1.0;
            break;
    }
}

TransitionEffect AnimatedIcon::transitionEffect() const
{
    return transitionEffect_;
}

void AnimatedIcon::setTransitionEffect(TransitionEffect effect)
{
    transitionEffect_ = effect;
}

QPointF AnimatedIcon::rectCenter(const QRect& rect)
{
    return QPointF(rect.left() + rect.width() * 0.5, rect.top() + rect.height() * 0.5);
}

} // namespace fluvel