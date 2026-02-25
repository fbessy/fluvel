#include "autofit_behavior.hpp"
#include "image_view.hpp"

#include <QMouseEvent>

namespace ofeli_app
{

AutoFitBehavior::AutoFitBehavior(Qt::MouseButton button)
    : button_(button)
{
}

bool AutoFitBehavior::mouseRelease(ImageView& view, QMouseEvent* event)
{
    if (event->button() == button_)
    {
        view.applyAutoFit();
        event->accept();
        return true;
    }

    return false;
}

} // namespace ofeli_app
