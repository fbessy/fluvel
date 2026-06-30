// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QIcon>
#include <QObject>
#include <QPointer>
#include <QRect>
#include <QSize>

class QPainter;
class QPropertyAnimation;
class QWidget;

namespace fluvel
{

/**
 * @brief Animates transitions between two icons.
 *
 * AnimatedIcon renders a smooth transition between two icons using
 * a perspective-like flip animation combined with a cross-fade.
 *
 * The class is independent from any specific button type and can be
 * reused by QToolButton, QPushButton or any custom widget able to
 * delegate icon painting.
 */
class AnimatedIcon : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal transitionProgress READ transitionProgress WRITE setTransitionProgress)

public:
    /**
     * @brief Direction of the flip animation.
     */
    enum class FlipDirection
    {
        /// Alternate the flip direction automatically.
        Auto,

        /// Always flip to the left.
        Left,

        /// Always flip to the right.
        Right
    };

    /**
     * @brief Constructs an animated icon renderer.
     *
     * @param owner Widget updated while the animation is running.
     */
    explicit AnimatedIcon(QWidget* owner);

    /**
     * @brief Draws the current icon or the animated transition.
     *
     * If no transition is active, @p defaultIcon is rendered.
     * Otherwise the internal animation state is used to draw
     * the outgoing and incoming icons.
     *
     * @param painter Painter used for rendering.
     * @param rect Drawing rectangle.
     * @param iconSize Requested icon size.
     * @param defaultIcon Icon displayed when no animation is active.
     */
    void paint(QPainter& painter, const QRect& rect, const QSize& iconSize,
               const QIcon& defaultIcon) const;

    /**
     * @brief Starts an animated transition between two icons.
     *
     * @param current Currently displayed icon.
     * @param next Target icon.
     * @param direction Flip direction.
     */
    void setAnimatedIcon(const QIcon& current, const QIcon& next,
                         FlipDirection direction = FlipDirection::Auto);

    /**
     * @brief Returns whether a transition is currently running.
     */
    bool isAnimating() const;

private:
    /**
     * @brief Returns the animation progress.
     */
    qreal transitionProgress() const;

    /**
     * @brief Updates the animation progress.
     *
     * This function is driven automatically by
     * QPropertyAnimation.
     *
     * @param progress Animation progress in the range [0, 1].
     */
    void setTransitionProgress(qreal progress);

    /**
     * @brief Updates the flip direction.
     *
     * @param direction Requested flip direction.
     */
    void updateTransitionDirection(FlipDirection direction);

private:
    /// Widget repainted during the animation.
    QPointer<QWidget> owner_;

    /// Icon leaving the screen.
    QIcon currentIcon_;

    /// Icon entering the screen.
    QIcon nextIcon_;

    /// Animation progress in the range [0, 1].
    qreal transitionProgress_{0.0};

    /// Horizontal flip direction.
    qreal transitionDirection_{1.0};

    /// Transition animation.
    QPointer<QPropertyAnimation> animation_;
};

} // namespace fluvel