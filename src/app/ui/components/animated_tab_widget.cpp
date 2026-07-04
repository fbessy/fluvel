// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "animated_tab_widget.hpp"

#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

namespace
{

//--------------------------------------------------
// Slide animation
//--------------------------------------------------

/// Slide animation duration in milliseconds.
constexpr int kSlideDurationMs = 100;

/// Horizontal slide offset in pixels.
constexpr int kSlideDistancePx = 10;

/// Easing curve used by the slide animation.
constexpr QEasingCurve::Type kSlideEasing = QEasingCurve::OutCubic;

//--------------------------------------------------
// Fade animation
//--------------------------------------------------

/// Fade animation duration in milliseconds.
constexpr int kFadeDurationMs = 100;

/// Initial page opacity.
constexpr qreal kFadeStartOpacity = 0.90;

/// Final page opacity.
constexpr qreal kFadeEndOpacity = 1.0;

/// Easing curve used by the fade animation.
constexpr QEasingCurve::Type kFadeEasing = QEasingCurve::OutCubic;

} // namespace

namespace fluvel
{

AnimatedTabWidget::AnimatedTabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    connect(this, &QTabWidget::currentChanged, this, &AnimatedTabWidget::animateCurrentPage);
}

void AnimatedTabWidget::animateCurrentPage(int index)
{
    QWidget* page = widget(index);

    if (!page)
        return;

    const QPoint finalPos = page->pos();

    //
    // Slide animation
    //
    page->move(finalPos.x() + kSlideDistancePx, finalPos.y());

    auto* slideAnimation = new QPropertyAnimation(page, "pos");

    slideAnimation->setDuration(kSlideDurationMs);
    slideAnimation->setStartValue(page->pos());
    slideAnimation->setEndValue(finalPos);
    slideAnimation->setEasingCurve(kSlideEasing);

    //
    // Fade animation
    //
    auto* effect = new QGraphicsOpacityEffect(page);

    page->setGraphicsEffect(effect);

    effect->setOpacity(kFadeStartOpacity);

    auto* fadeAnimation = new QPropertyAnimation(effect, "opacity");

    fadeAnimation->setDuration(kFadeDurationMs);
    fadeAnimation->setStartValue(kFadeStartOpacity);
    fadeAnimation->setEndValue(kFadeEndOpacity);
    fadeAnimation->setEasingCurve(kFadeEasing);

    //
    // Run both animations together
    //
    auto* animationGroup = new QParallelAnimationGroup(this);

    animationGroup->addAnimation(slideAnimation);
    animationGroup->addAnimation(fadeAnimation);

    connect(animationGroup, &QParallelAnimationGroup::finished, effect, &QObject::deleteLater);

    animationGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace fluvel