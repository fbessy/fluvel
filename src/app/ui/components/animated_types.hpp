// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

namespace fluvel
{

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

    /// Animates toward the left.
    Left,

    /// Animates toward the right.
    Right
};

/**
 * @brief Click feedback animation.
 *
 * This animation is played when the user presses the button.
 * It is independent from icon transition animations and affects
 * the whole button, including both the icon and the text.
 */
enum class ClickAnimation
{
    /// No click animation.
    None,

    /// Brief scale animation providing a press feedback.
    Scale
};

} // namespace fluvel