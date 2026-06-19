// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "jump_slider.hpp"

#include <QMouseEvent>
#include <QWidget>

#include <algorithm>

namespace fluvel
{

JumpSlider::JumpSlider(QWidget* parent, Qt::Orientation orientation)
    : QSlider(orientation, parent)
{
    setMouseTracking(true);
}

void JumpSlider::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        double ratio = 0.0;

        if (orientation() == Qt::Horizontal)
        {
            ratio = event->position().x() / width();
        }
        else
        {
            // Qt places the minimum value at the bottom for vertical sliders.
            ratio = 1.0 - (event->position().y() / height());
        }

        ratio = std::clamp(ratio, 0.0, 1.0);

        setValue(minimum() + ratio * (maximum() - minimum()));
    }

    QSlider::mousePressEvent(event);
}

} // namespace fluvel