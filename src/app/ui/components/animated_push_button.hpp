// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "animated_icon.hpp"
#include "scale_animation.hpp"

#include <QPushButton>

namespace fluvel
{

/**
 * @brief Push button supporting animated interactions.
 *
 * AnimatedPushButton behaves like a standard QPushButton while
 * providing smooth icon transitions and optional click feedback
 * animations.
 *
 * Icon rendering is delegated to AnimatedIcon, while click
 * feedback is handled independently by ScaleAnimation.
 * Both effects can be enabled or disabled separately.
 */
class AnimatedPushButton : public QPushButton
{
    Q_OBJECT

public:
    /**
     * @brief Alias for the icon transition direction.
     */
    using TransitionDirection = AnimatedIcon::TransitionDirection;

    /**
     * @brief Alias for the icon transition effect.
     */
    using TransitionEffect = AnimatedIcon::TransitionEffect;

    /**
     * @brief Constructs an animated push button.
     *
     * @param parent Parent widget.
     */
    explicit AnimatedPushButton(QWidget* parent = nullptr);

    /**
     * @brief Changes the button icon using an animated transition.
     *
     * The visual effect depends on the currently selected
     * transition effect.
     *
     * @param icon Target icon.
     * @param direction Transition direction.
     */
    void setAnimatedIcon(const QIcon& icon,
                         TransitionDirection direction = TransitionDirection::Auto);

    /**
     * @brief Returns the current transition effect.
     */
    [[nodiscard]]
    AnimatedPushButton::TransitionEffect transitionEffect() const;

    /**
     * @brief Sets the transition effect.
     *
     * @param effect New transition effect.
     */
    void setTransitionEffect(AnimatedPushButton::TransitionEffect effect);

    /**
     * @brief Returns the current click animation.
     */
    [[nodiscard]]
    ClickAnimation clickAnimation() const;

    /**
     * @brief Sets the click animation.
     *
     * @param animation Click animation to play when the button is pressed.
     */
    void setClickAnimation(ClickAnimation animation);

protected:
    /**
     * @brief Draws the push button.
     *
     * The button itself is rendered using the current Qt style,
     * while icon rendering is delegated to AnimatedIcon to support
     * animated transitions.
     */
    void paintEvent(QPaintEvent*) override;

    /**
     * @brief Handles mouse press events.
     *
     * Starts the optional click feedback animation before
     * forwarding the event to QPushButton.
     */
    void mousePressEvent(QMouseEvent* event) override;

private:
    /// Animated icon renderer.
    AnimatedIcon animatedIcon_;

    /// Click feedback animation.
    ScaleAnimation scaleAnimation_{this};

    /// Current click feedback animation.
    ClickAnimation clickAnimation_{ClickAnimation::Scale};
};

} // namespace fluvel