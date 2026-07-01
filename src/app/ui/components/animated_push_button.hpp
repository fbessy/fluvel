// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "animated_icon.hpp"

#include <QPushButton>

namespace fluvel
{

/**
 * @brief Push button supporting animated icon transitions.
 *
 * AnimatedPushButton behaves like a standard QPushButton while
 * adding smooth animated transitions between icons.
 *
 * Icon rendering is delegated to AnimatedIcon, allowing the
 * button to display a perspective-like flip animation combined
 * with a cross-fade whenever the icon changes.
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

protected:
    /**
     * @brief Draws the push button.
     *
     * The button itself is rendered using the current Qt style,
     * while icon rendering is delegated to AnimatedIcon to support
     * animated transitions.
     */
    void paintEvent(QPaintEvent*) override;

private:
    /// Animated icon renderer.
    AnimatedIcon animatedIcon_;
};

} // namespace fluvel