// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "jump_slider.hpp"

#include <QMouseEvent>
#include <QWidget>

#include <algorithm>

namespace fluvel
{

JumpSlider::JumpSlider(QWidget* parent)
    : QSlider(Qt::Horizontal, parent)
{
    setMouseTracking(true);
}

void JumpSlider::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const double ratio =
            std::clamp(static_cast<double>(event->position().x()) / width(), 0.0, 1.0);

        setValue(minimum() + ratio * (maximum() - minimum()));
    }

    QSlider::mousePressEvent(event);
}

} // namespace fluvel