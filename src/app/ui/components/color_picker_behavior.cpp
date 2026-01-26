#include "color_picker_behavior.hpp"

namespace ofeli_app
{

ColorPickerBehavior::ColorPickerBehavior(bool singleShot)
    : singleShot_(singleShot)
{
}

void ColorPickerBehavior::mousePress(ImageView& view, QMouseEvent* e)
{
    if (e->button() != Qt::LeftButton)
        return;

    QPoint imgPos = view.imageCoordinatesFromView(e->pos());
    if (imgPos.x() < 0)
        return;

    QColor color = view.pixelColorAt(imgPos);
    if (!color.isValid())
        return;

    if (onColorPicked)
        onColorPicked(color, imgPos);

    e->accept();

    //if (singleShot_)
        //view.removeBehavior(this); // ou désactivation externe
}

}
