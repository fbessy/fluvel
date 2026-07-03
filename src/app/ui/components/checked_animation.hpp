// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QObject>
#include <QPointer>

class QWidget;
class QPropertyAnimation;

namespace fluvel
{

/**
 * @brief Animation played when the checked state changes.
 */
enum class CheckAnimation
{
    /// No checked-state animation.
    None,

    /// Brief pop animation emphasizing state changes.
    Pop
};

/**
 * @brief Animated feedback for checkable widgets.
 *
 * CheckedAnimation provides a short scale animation when the
 * checked state of a widget changes.
 *
 * Unlike ScaleAnimation, which provides a press feedback,
 * this animation emphasizes state transitions such as
 * checked and unchecked.
 *
 * The animation is independent from icon transitions and can
 * be reused by any custom widget implementing its own
 * paintEvent().
 */
class CheckedAnimation : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal scale READ scale WRITE setScale)

public:
    /**
     * @brief Constructs a checked-state animation.
     *
     * @param owner Widget updated during the animation.
     */
    explicit CheckedAnimation(QWidget* owner);

    /**
     * @brief Starts the checked-state animation.
     *
     * Expands the widget before returning to its resting size.
     */
    void check();

    /**
     * @brief Starts the unchecked-state animation.
     *
     * Briefly shrinks the widget before returning to its resting size.
     */
    void uncheck();

    /**
     * @brief Returns the current scale factor.
     */
    [[nodiscard]]
    qreal scale() const;

private:
    /**
     * @brief Starts a state transition animation.
     *
     * The animation interpolates between the given scale values
     * using keyframes.
     *
     * @param start Initial scale factor.
     * @param peak Intermediate overshoot or undershoot scale.
     * @param end Final resting scale.
     */
    void animate(qreal start, qreal peak, qreal end);

    /**
     * @brief Updates the current scale factor.
     *
     * Called automatically by Qt's property animation system.
     */
    void setScale(qreal scale);

private:
    QPointer<QWidget> owner_;
    QPointer<QPropertyAnimation> animation_;

    qreal scale_{1.0};
};

} // namespace fluvel