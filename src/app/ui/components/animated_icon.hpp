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
 * AnimatedIcon renders animated transitions between icons while
 * preserving the visual state (enabled/disabled and checked/unchecked)
 * of the owning widget.
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
     * @brief Visual effect used for icon transitions.
     */
    enum class TransitionEffect
    {
        /// Perspective-like flip animation.
        Flip,

        /// Horizontal slide animation.
        Slide
    };

    /**
     * @brief Direction of the transition animation.
     */
    enum class TransitionDirection
    {
        /// Alternate the transition direction automatically.
        Auto,

        /// Always animate toward the left.
        Left,

        /// Always animate toward the right.
        Right
    };

    /**
     * @brief Constructs an animated icon renderer.
     *
     * @param owner Widget updated while the animation is running.
     */
    explicit AnimatedIcon(QWidget* owner);

    /**
     * @brief Returns the current transition effect.
     */
    TransitionEffect transitionEffect() const;

    /**
     * @brief Sets the transition effect.
     *
     * @param effect New transition effect.
     */
    void setTransitionEffect(TransitionEffect effect);

    /**
     * @brief Draws the current icon or the animated transition.
     *
     * If no transition is active, @p defaultIcon is rendered.
     * Otherwise the internal animation state is used to draw
     * the outgoing and incoming icons.
     *
     * The icon is rendered using the current visual state of the
     * owner widget (enabled/disabled and checked/unchecked).
     *
     * @param painter Painter used for rendering.
     * @param rect Drawing rectangle.
     * @param iconSize Requested icon size.
     * @param defaultIcon Icon displayed when no animation is active.
     */
    void paint(QPainter& painter, const QRect& rect, const QSize& iconSize,
               const QIcon& defaultIcon);

    /**
     * @brief Starts an animated transition between two icons.
     *
     * @param current Currently displayed icon.
     * @param next Target icon.
     * @param direction Transition direction.
     */
    void setAnimatedIcon(const QIcon& current, const QIcon& next,
                         TransitionDirection direction = TransitionDirection::Auto);

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
     * @brief Updates the transition direction.
     */
    void updateTransitionDirection(TransitionDirection direction);

private:
    /**
     * @brief Paints the flip transition.
     */
    void paintFlip(QPainter& painter, const QRect& rect, const QSize& iconSize,
                   const QIcon& defaultIcon);

    /**
     * @brief Paints the slide transition.
     */
    void paintSlide(QPainter& painter, const QRect& rect, const QSize& iconSize,
                    const QIcon& defaultIcon);

    /**
     * @brief Draws an icon with the requested transform.
     *
     * The icon variant is selected automatically according to the
     * current state of the owner widget (enabled/disabled and
     * checked/unchecked).
     *
     * @param painter Painter used for rendering.
     * @param center Icon center.
     * @param iconSize Requested icon size.
     * @param icon Icon to draw.
     * @param opacity Icon opacity.
     * @param angle Rotation angle in degrees.
     * @param offsetX Horizontal translation.
     * @param scale Additional scale factor.
     */
    void drawIcon(QPainter& painter, const QPointF& center, const QSize& iconSize,
                  const QIcon& icon, qreal opacity, qreal angle, qreal offsetX, qreal scale);

    static QPointF rectCenter(const QRect& rect);

    /// Owner widget used for repaint requests and icon state.
    QPointer<QWidget> owner_;

    /// Icon leaving the screen.
    QIcon currentIcon_;

    /// Icon entering the screen.
    QIcon nextIcon_;

    /// Animation progress in the range [0, 1].
    qreal transitionProgress_{0.0};

    /// Direction used by transition effects.
    qreal transitionDirection_{-1.0};

    /// Transition animation.
    QPointer<QPropertyAnimation> animation_;

    /// Visual effect used for icon transitions.
    TransitionEffect transitionEffect_{TransitionEffect::Slide};
};

} // namespace fluvel