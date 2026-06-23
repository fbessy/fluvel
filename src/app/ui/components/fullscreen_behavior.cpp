// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "fullscreen_behavior.hpp"
#include "image_viewer_widget.hpp"

#include <QMouseEvent>

namespace fluvel
{

bool FullscreenBehavior::mouseDoubleClick(ImageViewerWidget& view, QMouseEvent* event)
{
    Q_UNUSED(event);

    if (event->type() == QEvent::MouseButtonDblClick && event->button() == Qt::LeftButton &&
        view.hasImage())
    {
        view.toggleFullscreen();
        event->accept();

        return true;
    }

    return false;
}

Qt::CursorShape FullscreenBehavior::availableCursor(bool, bool, const ImageViewerWidget& view,
                                                    const QMouseEvent*) const
{
    if (view.isFullscreen() && !view.isUserActive())
        return Qt::BlankCursor;

    return Qt::ArrowCursor;
}

} // namespace fluvel
