// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "fullscreen_behavior.hpp"
#include "image_view.hpp"

#include <QMouseEvent>

namespace fluvel_app
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

} // namespace fluvel_app
