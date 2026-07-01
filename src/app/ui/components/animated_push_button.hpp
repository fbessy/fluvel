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
     * @brief Alias for the icon flip direction.
     */
    using FlipDirection = AnimatedIcon::FlipDirection;

    /**
     * @brief Constructs an animated push button.
     *
     * @param parent Parent widget.
     */
    explicit AnimatedPushButton(QWidget* parent = nullptr);

    /**
     * @brief Changes the button icon using an animated transition.
     *
     * The current icon smoothly transitions to the new icon using
     * a perspective-like flip animation combined with a cross-fade.
     *
     * @param icon Target icon.
     * @param direction Flip direction.
     */
    void setAnimatedIcon(const QIcon& icon, FlipDirection direction = FlipDirection::Auto);

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