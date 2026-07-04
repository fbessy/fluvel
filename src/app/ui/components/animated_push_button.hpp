// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "animated_icon.hpp"
#include "animated_types.hpp"
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
     * @brief Constructs an animated push button.
     *
     * @param parent Parent widget.
     */
    explicit AnimatedPushButton(QWidget* parent = nullptr);

    /**
     * @brief Constructs an animated push button with text.
     *
     * @param text Button text.
     * @param parent Parent widget.
     */
    explicit AnimatedPushButton(const QString& text, QWidget* parent = nullptr);

    /**
     * @brief Constructs an animated push button with an icon and text.
     *
     * @param icon Button icon.
     * @param text Button text.
     * @param parent Parent widget.
     */
    explicit AnimatedPushButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);

    /**
     * @brief Changes the button icon and starts an animated transition.
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
    TransitionEffect transitionEffect() const;

    /**
     * @brief Sets the transition effect.
     *
     * @param effect New transition effect.
     */
    void setTransitionEffect(TransitionEffect effect);

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
     * The button bevel is rendered using the current Qt style,
     * while the icon and text are laid out and painted separately
     * to support animated icon rendering.
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
    /**
     * @brief Layout information used to render a push button.
     */
    struct ButtonLayout
    {
        /// Button contents rectangle.
        QRect contentsRect;

        /// Icon drawing rectangle.
        QRect iconRect;

        /// Text drawing rectangle.
        QRect textRect;
    };

    /**
     * @brief Computes the layout of the button contents.
     *
     * Calculates the rectangles used to render the icon and the text
     * according to the current button state and style metrics.
     *
     * @return Button layout information.
     */
    ButtonLayout calculateLayout() const;

    /// Animated icon renderer.
    AnimatedIcon animatedIcon_;

    /// Click feedback animation.
    ScaleAnimation scaleAnimation_{this};

    /// Current click feedback animation.
    ClickAnimation clickAnimation_{ClickAnimation::Scale};
};

} // namespace fluvel