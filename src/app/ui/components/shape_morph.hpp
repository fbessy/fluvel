// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QColor>
#include <QEasingCurve>
#include <QObject>
#include <QVariantAnimation>

namespace fluvel
{

/**
 * @brief Rounded-square symbol described by ratios, independent of any size.
 *
 * A radius ratio of 0.5 gives a disk, a small ratio a rounded square.
 */
struct MorphShape
{
    /// Symbol side relative to the available square (0..1).
    qreal sizeRatio{0.0};

    /// Corner radius relative to the symbol side (0..0.5).
    qreal radiusRatio{0.0};

    /// Fill color. An invalid color means "use the widget text color"
    /// (resolved by the widget, not by ShapeMorph).
    QColor color;
};

/**
 * @brief Interpolates smoothly from the displayed shape toward a target shape.
 *
 * ShapeMorph only knows about shapes. It does not know about states, buttons
 * or painting: the owner reads shape() when painting and repaints on
 * shapeChanged().
 *
 * When the target changes during a transition, the new transition starts from
 * the shape currently displayed, so there is no visual jump.
 */
class ShapeMorph : public QObject
{
    Q_OBJECT

public:
    explicit ShapeMorph(QObject* parent = nullptr);

    void setDuration(int milliseconds);
    void setEasingCurve(const QEasingCurve& curve);

    /// Shape to display right now.
    [[nodiscard]]
    MorphShape shape() const;

    /// Starts a transition from the displayed shape to @p target.
    void animateTo(const MorphShape& target);

    /// Stops any transition and displays @p shape immediately.
    void jumpTo(const MorphShape& shape);

signals:
    void shapeChanged();

private:
    MorphShape from_;
    MorphShape to_;
    qreal progress_{1.0};

    QVariantAnimation animation_;
};

} // namespace fluvel
