#include "zoom_behavior.hpp"
#include "image_view.hpp"

namespace ofeli_app {

ZoomBehavior::ZoomBehavior(double minZoom,
                           double maxZoom,
                           double zoomFactor)
    : minZoom_(minZoom)
    , maxZoom_(maxZoom)
    , zoomFactor_(zoomFactor)
{
}

void ZoomBehavior::wheel(ImageView& view,
                         QWheelEvent* event)
{
    /*
    const int delta = event->angleDelta().y();
    if (!delta) return;

    constexpr double f = 1.15;
    view.zoomAt(event->position().toPoint(),
                delta > 0 ? f : 1.0 / f);
*/
    event->accept();
}

} // namespace ofeli_app
