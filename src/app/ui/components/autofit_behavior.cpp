#include "autofit_behavior.hpp"
#include "image_view.hpp"

#include <QMouseEvent>

namespace ofeli_app
{

AutoFitBehavior::AutoFitBehavior(Qt::MouseButton button)
    : button_(button)
{
}

void AutoFitBehavior::mouseRelease(ImageView& view,
                                   QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton)
    {
        view.applyAutoFit();
        event->accept();
    }
}

}
