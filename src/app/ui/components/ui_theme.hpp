// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file ui_theme.hpp
 * @brief Shared UI theme definitions including colors, metrics, spacing and
 * animation durations.
 */

#pragma once

#include <QColor>

/**
 * @brief Shared user interface theme constants.
 *
 * This namespace centralizes the visual appearance of the application,
 * including colors, dimensions, spacing and animation timings used by
 * reusable widgets.
 */
namespace fluvel::ui
{

// ============================================================================
// Accent colors
// ============================================================================

inline const QColor kAccentColor(107, 111, 207, 220);
inline const QColor kAccentHoverColor = kAccentColor.lighter(110);

// ============================================================================
// Text colors
// ============================================================================

inline const QColor kTextColor(Qt::white);
inline const QColor kDisabledTextColor(255, 255, 255, 70);
inline const QColor kPopupBorderColor(60, 60, 60);

// ============================================================================
// Panel colors
// ============================================================================

inline const QColor kPanelBackground(32, 36, 42, 210);

// ============================================================================
// Generic control colors
// ============================================================================

inline const QColor kControlBackground(255, 255, 255, 20);
inline const QColor kControlHover(255, 255, 255, 30);
inline const QColor kControlPressed(255, 255, 255, 38);

inline const QColor kControlBorder(255, 255, 255, 35);

inline const QColor kControlDisabled(255, 255, 255, 8);
inline const QColor kControlDisabledBorder(255, 255, 255, 10);

// ============================================================================
// Selection colors
// ============================================================================

inline const QColor kSelectionBackground(255, 255, 255, 25);
inline const QColor kSelectionHover(255, 255, 255, 15);

// ============================================================================
// Slider colors
// ============================================================================

inline const QColor kSliderGrooveColor(255, 255, 255, 80);
inline const QColor kSliderProgressColor = kAccentColor;
inline const QColor kSliderHandleColor(Qt::white);
inline const QColor kSliderHandleBorderColor(kAccentColor.red(), kAccentColor.green(),
                                             kAccentColor.blue(), 120);

inline const QColor kTooltipBackgroundColor(20, 20, 20, 180);
inline const QColor kTooltipTextColor = kTextColor;

// ============================================================================
// Metrics
// ============================================================================

constexpr int kButtonSize = 34;
constexpr int kButtonIconSize = 22;

constexpr int kControlRadius = 15;
constexpr int kPopupRadius = 12;
constexpr int kPanelRadius = 16;

// ============================================================================
// Layout
// ============================================================================

constexpr int kControlSpacing = 6;
constexpr int kGroupSpacing = 14;

// ============================================================================
// Animations
// ============================================================================

constexpr int kFastAnimationMs = 100;
constexpr int kNormalAnimationMs = 200;
constexpr int kSlowAnimationMs = 300;

} // namespace fluvel::ui