// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "animated_icon.hpp"
#include "checked_animation.hpp"
#include "scale_animation.hpp"
#include "ui_appearance.hpp"

#include <QToolButton>

namespace fluvel
{

/**
 * @brief Tool button supporting Fluvel visual styles.
 *
 * The widget provides either the native Qt appearance or the
 * custom Fluvel modern appearance used by fullscreen controls.
 */
class StyledToolButton : public QToolButton
{
    Q_OBJECT

public:
    using TransitionEffect = AnimatedIcon::TransitionEffect;
    using TransitionDirection = AnimatedIcon::TransitionDirection;

    /**
     * @brief Constructs a styled tool button.
     *
     * @param parent Parent widget.
     * @param appearance Desired appearance.
     */
    explicit StyledToolButton(QWidget* parent = nullptr,
                              ui::Appearance appearance = ui::Appearance::Modern);

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
     * @brief Returns the current appearance.
     */
    [[nodiscard]]
    ui::Appearance appearance() const;

    /**
     * @brief Changes the button appearance.
     */
    void setAppearance(ui::Appearance appearance);

    /**
     * @brief Returns the current transition effect.
     */
    [[nodiscard]]
    StyledToolButton::TransitionEffect transitionEffect() const;

    /**
     * @brief Sets the transition effect.
     *
     * @param effect New transition effect.
     */
    void setTransitionEffect(StyledToolButton::TransitionEffect effect);

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

    /**
     * @brief Returns the current checked-state animation.
     */
    [[nodiscard]]
    CheckAnimation checkAnimation() const;

    /**
     * @brief Sets the checked-state animation.
     *
     * Enabling a checked-state animation automatically disables
     * the click animation, since both effects are mutually exclusive.
     *
     * @param animation Checked-state animation.
     */
    void setCheckAnimation(CheckAnimation animation);

protected:
    /**
     * @brief Draws the styled button.
     *
     * The button frame is rendered using the current Qt style,
     * while icon rendering is delegated to AnimatedIcon to support
     * animated transitions.
     */
    void paintEvent(QPaintEvent*) override;

    /**
     * @brief Handles mouse press events.
     *
     * Starts the optional click feedback animation before
     * forwarding the event to QToolButton.
     */
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * @brief Handles checked-state changes.
     *
     * Starts the optional checked-state animation before
     * forwarding the state change to QToolButton.
     */
    void nextCheckState() override;

private:
    /**
     * @brief Applies the current visual appearance.
     */
    void updateStyle();

    /// Current visual appearance.
    ui::Appearance appearance_;

    /// Animated icon renderer.
    AnimatedIcon animatedIcon_;

    /// Click feedback animation.
    ScaleAnimation scaleAnimation_{this};

    /// Selected click feedback effect.
    ClickAnimation clickAnimation_{ClickAnimation::Scale};

    /// Checked-state feedback animation.
    CheckedAnimation checkedAnimation_{this};

    /// Current checked-state animation.
    CheckAnimation checkAnimation_{CheckAnimation::None};
};

} // namespace fluvel