// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "shape_morph.hpp"

namespace
{

constexpr int kDefaultDurationMs = 280;

qreal lerp(qreal a, qreal b, qreal t)
{
    return a + (b - a) * t;
}

QColor lerp(const QColor& a, const QColor& b, qreal t)
{
    return QColor::fromRgbF(lerp(a.redF(), b.redF(), t), lerp(a.greenF(), b.greenF(), t),
                            lerp(a.blueF(), b.blueF(), t), lerp(a.alphaF(), b.alphaF(), t));
}

} // namespace

namespace fluvel
{

ShapeMorph::ShapeMorph(QObject* parent)
    : QObject(parent)
    , animation_(this)
{
    animation_.setDuration(kDefaultDurationMs);
    animation_.setEasingCurve(QEasingCurve::InOutCubic);
    animation_.setStartValue(0.0);
    animation_.setEndValue(1.0);

    connect(&animation_, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& value)
            {
                progress_ = value.toReal();
                emit shapeChanged();
            });
}

void ShapeMorph::setDuration(int milliseconds)
{
    animation_.setDuration(milliseconds);
}

void ShapeMorph::setEasingCurve(const QEasingCurve& curve)
{
    animation_.setEasingCurve(curve);
}

MorphShape ShapeMorph::shape() const
{
    return {lerp(from_.sizeRatio, to_.sizeRatio, progress_),
            lerp(from_.radiusRatio, to_.radiusRatio, progress_),
            lerp(from_.color, to_.color, progress_)};
}

void ShapeMorph::animateTo(const MorphShape& target)
{
    // Must be computed before to_ is replaced.
    from_ = shape();
    to_ = target;
    progress_ = 0.0;

    animation_.stop();
    animation_.start();
}

void ShapeMorph::jumpTo(const MorphShape& shape)
{
    animation_.stop();

    from_ = shape;
    to_ = shape;
    progress_ = 1.0;

    emit shapeChanged();
}

} // namespace fluvel
