#include "color_picker_behavior.hpp"
#include "image_view.hpp"

namespace ofeli_app
{

ColorPickerBehavior::ColorPickerBehavior(Qt::MouseButton button)
    : button_(button)
{
}

void ColorPickerBehavior::mousePress(ImageView& view, QMouseEvent* e)
{
    if (e->button() != button_)
        return;

    QPoint imgPos = view.imageCoordinatesFromView(e->pos());
    if (imgPos.x() < 0)
        return;

    QColor color = view.pixelColorAt(imgPos);
    if (!color.isValid())
        return;

    view.onColorPicked(color, imgPos);

    e->accept();
}

}
