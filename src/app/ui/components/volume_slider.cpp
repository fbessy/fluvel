// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "volume_slider.hpp"
#include "ui_theme.hpp"

namespace fluvel
{

VolumeSlider::VolumeSlider(QWidget* parent, ui::Appearance appearance)
    : StyledSlider(parent, appearance)
{
    // TODO: Remove once SliderStyle is introduced.
    if (isModernStyle())
        setFixedHeight(ui::kVolumeSliderHeight);
}

QString VolumeSlider::hoverText(double ratio) const
{
    const int volume = minimum() + ratio * (maximum() - minimum());

    return QString("%1 %").arg(volume);
}

int VolumeSlider::grooveHeight() const
{
    return 4;
}

int VolumeSlider::grooveHoverHeight() const
{
    return 6;
}

int VolumeSlider::handleRadius() const
{
    return 7;
}

int VolumeSlider::handleHoverRadius() const
{
    return 9;
}

int VolumeSlider::topMargin() const
{
    return 8;
}

int VolumeSlider::bottomMargin() const
{
    return 4;
}

int VolumeSlider::sliderHeight() const
{
    return 58;
}

int VolumeSlider::hoverFontPointSize() const
{
    return 8;
}

QMargins VolumeSlider::hoverBubbleMargins() const
{
    return {6, 2, 6, 2};
}

int VolumeSlider::hoverBubbleRadius() const
{
    return 6;
}

int VolumeSlider::hoverBubbleOffset() const
{
    return 14;
}

} // namespace fluvel