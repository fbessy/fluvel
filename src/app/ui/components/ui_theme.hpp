// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QColor>

namespace fluvel::ui
{

// ============================================================================
// Accent colors
// ============================================================================

inline const QColor kAccentColor(107, 111, 207, 220);
inline const QColor kAccentHover = kAccentColor.lighter(110);

// ============================================================================
// Text colors
// ============================================================================

inline const QColor kTextColor(Qt::white);
inline const QColor kDisabledTextColor(255, 255, 255, 70);

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
// Metrics
// ============================================================================

constexpr int kButtonSize = 34;
constexpr int kButtonIconSize = 24;

constexpr int kControlRadius = 15;
constexpr int kPopupRadius = 12;
constexpr int kPanelRadius = 16;

constexpr int kSliderHeight = 82;

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