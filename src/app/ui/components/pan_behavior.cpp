// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "pan_behavior.hpp"
#include "image_view.hpp"

#include <QMouseEvent>

namespace fluvel_app
{

PanBehavior::PanBehavior(Qt::MouseButton button)
    : button_(button)
{
}

bool PanBehavior::mousePress(ImageView& view, QMouseEvent* e)
{
    if (e->button() == button_ && view.isPanRelevant())
    {
        capturing_ = true;
        lastPos_ = e->pos();
        return true;
    }

    return false;
}

bool PanBehavior::mouseMove(ImageView& view, QMouseEvent* e)
{
    if (!capturing_ || !view.isPanRelevant())
        return false;

    QPoint delta = e->pos() - lastPos_;
    lastPos_ = e->pos();

    const qreal zoom = view.transform().m11();
    view.translateView(delta.x() / zoom, delta.y() / zoom);

    view.userInteracted();
    e->accept();

    return true;
}

bool PanBehavior::mouseRelease(ImageView&, QMouseEvent* e)
{
    if (e->button() == button_ && capturing_)
    {
        capturing_ = false;
        e->accept();
        return true;
    }

    return false;
}
} // namespace fluvel_app
