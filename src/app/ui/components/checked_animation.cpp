// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "checked_animation.hpp"

#include <QEasingCurve>
#include <QPropertyAnimation>
#include <QWidget>

namespace
{

constexpr int kAnimationDurationMs = 190;

/// Normalized animation time at which the overshoot is reached.
constexpr qreal kPeakTime = 0.60;

constexpr qreal kCheckPeakScale = 1.14;
constexpr qreal kUncheckPeakScale = 0.88;

} // namespace

namespace fluvel
{

CheckedAnimation::CheckedAnimation(QWidget* owner)
    : QObject(owner)
    , owner_(owner)
{
}

void CheckedAnimation::check()
{
    animate(1.0, kCheckPeakScale, 1.0);
}

void CheckedAnimation::uncheck()
{
    animate(1.0, kUncheckPeakScale, 1.0);
}

void CheckedAnimation::animate(qreal start, qreal peak, qreal end)
{
    if (animation_)
        animation_->stop();

    animation_ = new QPropertyAnimation(this, "scale");

    animation_->setDuration(kAnimationDurationMs);

    animation_->setKeyValueAt(0.0, start);
    animation_->setKeyValueAt(kPeakTime, peak);
    animation_->setKeyValueAt(1.0, end);

    animation_->setEasingCurve(QEasingCurve::OutCubic);

    connect(animation_, &QPropertyAnimation::finished, this,
            [this]
            {
                animation_ = nullptr;
                scale_ = 1.0;

                if (owner_)
                    owner_->update();
            });

    animation_->start(QAbstractAnimation::DeleteWhenStopped);
}

qreal CheckedAnimation::scale() const
{
    return scale_;
}

void CheckedAnimation::setScale(qreal scale)
{
    if (qFuzzyCompare(scale_, scale))
        return;

    scale_ = scale;

    if (owner_)
        owner_->update();
}

} // namespace fluvel