#include "pan_behavior.hpp"
#include "image_view.hpp"

#include <QMouseEvent>

namespace ofeli_app
{

PanBehavior::PanBehavior(Qt::MouseButton button)
    : button_(button)
{
}

bool PanBehavior::mousePress(ImageView&,
                             QMouseEvent* e)
{
    if (e->button() == button_)
    {
        capturing_ = true;
        lastPos_ = e->pos();
        // ❌ pas de accept ici

        return true;
    }

    return false;
}

bool PanBehavior::mouseMove(ImageView& view,
                            QMouseEvent* e)
{
    if (!capturing_)
        return false;

    QPoint delta = e->pos() - lastPos_;
    lastPos_ = e->pos();

    const qreal zoom = view.transform().m11(); // scale X
    view.translate(delta.x() / zoom,
                   delta.y() / zoom);

    view.userInteracted();
    e->accept();

    return true;
}

bool PanBehavior::mouseRelease(ImageView&,
                               QMouseEvent* e)
{
    if (e->button() == button_)
    {
        capturing_ = false;
        e->accept();

        return true;
    }

    return false;
}
} // namespace ofeli_app
