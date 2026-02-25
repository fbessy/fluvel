#include "fullscreen_behavior.hpp"
#include "image_view.hpp"

#include <QMouseEvent>

namespace ofeli_app
{

bool FullscreenBehavior::mouseDoubleClick(ImageView& view, QMouseEvent* event)
{
    Q_UNUSED(event);

    if (event->type() == QEvent::MouseButtonDblClick && event->button() == Qt::LeftButton)
    {
        view.toggleFullscreen();
        event->accept();

        return true;
    }

    return false;
}

} // namespace ofeli_app
