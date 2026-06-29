// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "timeline_slider.hpp"

#include "time_utils.hpp"

namespace fluvel
{

TimelineSlider::TimelineSlider(QWidget* parent, ui::Appearance appearance)
    : StyledSlider(parent, appearance, SliderStyle::timeline())
{
}

QString TimelineSlider::hoverText(double ratio) const
{
    const qint64 position = minimum() + ratio * (maximum() - minimum());

    return time_utils::formatDuration(position);
}

} // namespace fluvel