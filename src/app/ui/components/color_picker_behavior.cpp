// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "color_picker_behavior.hpp"
#include "image_view.hpp"

namespace fluvel_app
{

ColorPickerBehavior::ColorPickerBehavior(Qt::MouseButton button)
    : button_(button)
{
}

bool ColorPickerBehavior::mousePress(ImageView& view, QMouseEvent* e)
{
    if (e->button() != button_)
        return false;

    const QPoint imgPos = view.imageCoordinatesFromView(e->pos());
    if (imgPos.x() < 0)
        return false;

    const QColor color = view.pixelColorAt(imgPos);
    if (!color.isValid())
        return false;

    view.onColorPicked(color, imgPos);

    e->accept();
    return true;
}

} // namespace fluvel_app
