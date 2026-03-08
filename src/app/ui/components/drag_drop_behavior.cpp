// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "drag_drop_behavior.hpp"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>

namespace fluvel_app
{

bool DragDropBehavior::dragEnter(ImageView& view, QDragEnterEvent* e)
{
    if (!e->mimeData()->hasUrls())
        return false;

    view.setDragHighlight(true);

    e->acceptProposedAction();
    return true;
}

bool DragDropBehavior::dragMove(ImageView&, QDragMoveEvent* e)
{
    e->acceptProposedAction();
    return true;
}

bool DragDropBehavior::dragLeave(ImageView& view, QDragLeaveEvent*)
{
    view.setDragHighlight(false);
    return true;
}

bool DragDropBehavior::drop(ImageView& view, QDropEvent* e)
{
    view.setDragHighlight(false);

    const auto urls = e->mimeData()->urls();
    if (urls.isEmpty())
        return false;

    const QString path = urls.first().toLocalFile();

    view.notifyImageDropped(path);

    e->acceptProposedAction();
    return true;
}

} // namespace fluvel_app
