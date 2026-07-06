// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "scale_animation.hpp"

#include <QEasingCurve>
#include <QPropertyAnimation>
#include <QWidget>
#include <QtNumeric>

namespace
{

constexpr int kPressDurationMs = 140;

/// Normalized animation time at which the minimum scale is reached.
constexpr qreal kMinimumScaleTime = 0.4;

constexpr qreal kMinimumScale = 0.84;

} // namespace

namespace fluvel
{

ScaleAnimation::ScaleAnimation(QWidget* owner)
    : QObject(owner)
    , owner_(owner)
{
}

void ScaleAnimation::start()
{
    if (animation_)
        animation_->stop();

    animation_ = new QPropertyAnimation(this, "scale");

    animation_->setDuration(kPressDurationMs);

    animation_->setKeyValueAt(0.0, 1.0);
    animation_->setKeyValueAt(kMinimumScaleTime, kMinimumScale);
    animation_->setKeyValueAt(1.0, 1.0);

    animation_->setEasingCurve(QEasingCurve::OutQuad);

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

qreal ScaleAnimation::scale() const
{
    return scale_;
}

void ScaleAnimation::setScale(qreal scale)
{
    if (qFuzzyCompare(scale_, scale))
        return;

    scale_ = scale;

    if (owner_)
        owner_->update();
}

} // namespace fluvel