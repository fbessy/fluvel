#include "zoom_behavior.hpp"
#include "image_view.hpp"

namespace ofeli_app
{

ZoomBehavior::ZoomBehavior(double /*zoomFactor*/)
{
}

bool ZoomBehavior::wheel(ImageView& /*view*/, QWheelEvent* event)
{
    /*
    const int delta = event->angleDelta().y();
    if (!delta) return;

    constexpr double f = 1.15;
    view.zoomAt(event->position().toPoint(),
                delta > 0 ? f : 1.0 / f);
*/
    event->accept();
    return true;
}

} // namespace ofeli_app
