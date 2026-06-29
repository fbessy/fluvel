// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QColor>
#include <QMargins>

namespace fluvel
{

// ============================================================================
// Default timeline slider metrics
// ============================================================================

constexpr int kSliderHeight = 82;

constexpr int kSliderSideMargin = 16;
constexpr int kSliderTopMargin = 16;
constexpr int kSliderBottomMargin = 6;

constexpr int kSliderGrooveHeight = 8;
constexpr int kSliderGrooveHoverHeight = 11;

constexpr int kSliderHandleRadius = 11;
constexpr int kSliderHandleHoverRadius = 13;

constexpr int kSliderHoverFontPointSize = 10;
constexpr int kSliderHoverBubbleRadius = 8;
constexpr int kSliderHoverBubbleOffset = 22;

inline const QMargins kSliderHoverBubbleMargins{10, 4, 10, 4};

// ============================================================================
// Volume slider metrics
// ============================================================================

constexpr int kVolumeSliderHeight = 58;

constexpr int kVolumeSliderTopMargin = 8;
constexpr int kVolumeSliderBottomMargin = 4;

constexpr int kVolumeSliderGrooveHeight = 4;
constexpr int kVolumeSliderGrooveHoverHeight = 6;

constexpr int kVolumeSliderHandleRadius = 7;
constexpr int kVolumeSliderHandleHoverRadius = 9;

constexpr int kVolumeSliderHoverFontPointSize = 8;
constexpr int kVolumeSliderHoverBubbleRadius = 6;
constexpr int kVolumeSliderHoverBubbleOffset = 14;

inline const QMargins kVolumeSliderHoverBubbleMargins{6, 2, 6, 2};

/**
 * @brief Visual parameters of a StyledSlider.
 *
 * SliderStyle stores all geometry and rendering parameters used by
 * StyledSlider. It allows different slider variants to share the same
 * painting code while customizing only their appearance.
 *
 * The default constructor creates the standard timeline style.
 */
struct SliderStyle
{
    /**
     * @brief Constructs the default timeline slider style.
     */
    SliderStyle() = default;

    /**
     * @brief Creates the default timeline slider style.
     *
     * @return Timeline slider style.
     */
    static SliderStyle timeline()
    {
        return {};
    }

    /**
     * @brief Creates the compact style used by VolumeSlider.
     *
     * @return Volume slider style.
     */
    static SliderStyle volume()
    {
        SliderStyle style;

        style.sliderHeight = kVolumeSliderHeight;

        style.topMargin = kVolumeSliderTopMargin;
        style.bottomMargin = kVolumeSliderBottomMargin;

        style.grooveHeight = kVolumeSliderGrooveHeight;
        style.grooveHoverHeight = kVolumeSliderGrooveHoverHeight;

        style.handleRadius = kVolumeSliderHandleRadius;
        style.handleHoverRadius = kVolumeSliderHandleHoverRadius;

        style.hoverFontPointSize = kVolumeSliderHoverFontPointSize;
        style.hoverBubbleMargins = kVolumeSliderHoverBubbleMargins;
        style.hoverBubbleRadius = kVolumeSliderHoverBubbleRadius;
        style.hoverBubbleOffset = kVolumeSliderHoverBubbleOffset;

        return style;
    }

    int sliderHeight{kSliderHeight};

    int topMargin{kSliderTopMargin};
    int bottomMargin{kSliderBottomMargin};
    int sideMargin{kSliderSideMargin};

    int grooveHeight{kSliderGrooveHeight};
    int grooveHoverHeight{kSliderGrooveHoverHeight};

    int handleRadius{kSliderHandleRadius};
    int handleHoverRadius{kSliderHandleHoverRadius};

    int hoverFontPointSize{kSliderHoverFontPointSize};

    QMargins hoverBubbleMargins{kSliderHoverBubbleMargins};

    int hoverBubbleRadius{kSliderHoverBubbleRadius};
    int hoverBubbleOffset{kSliderHoverBubbleOffset};
};

} // namespace fluvel