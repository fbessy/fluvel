// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "drag_drop_behavior.hpp"
#include "file_utils.hpp"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QString>

namespace fluvel
{

DragDropBehavior::DragDropBehavior(DragDropContent content, const QString& placeholder)
    : content_(content)
    , placeholder_(placeholder)
{
}

bool DragDropBehavior::dragEnter(ImageViewerWidget& view, QDragEnterEvent* e)
{
    const auto urls = e->mimeData()->urls();

    if (urls.isEmpty())
        return false;

    const QString path = urls.first().toLocalFile();

    if (!acceptsFile(path))
        return false;

    view.setDragHighlight(true);

    e->acceptProposedAction();
    return true;
}

bool DragDropBehavior::dragMove(ImageViewerWidget& view, QDragMoveEvent* e)
{
    const auto urls = e->mimeData()->urls();

    if (urls.isEmpty())
        return false;

    const QString path = urls.first().toLocalFile();

    const bool accepted = acceptsFile(path);

    view.setDragHighlight(accepted);

    if (accepted)
        e->acceptProposedAction();

    return accepted;
}

bool DragDropBehavior::dragLeave(ImageViewerWidget& view, QDragLeaveEvent*)
{
    view.setDragHighlight(false);
    return true;
}

bool DragDropBehavior::drop(ImageViewerWidget& view, QDropEvent* e)
{
    view.setDragHighlight(false);

    const auto urls = e->mimeData()->urls();
    if (urls.isEmpty())
        return false;

    const QString path = urls.first().toLocalFile();

    QTimer::singleShot(0, &view,
                       [path, &view]()
                       {
                           view.notifyImageDropped(path);
                       });

    e->acceptProposedAction();
    return true;
}

bool DragDropBehavior::acceptsFile(const QString& filename) const
{
    switch (content_)
    {
        case DragDropContent::Images:
            return file_utils::isSupportedImage(filename);

        case DragDropContent::Videos:
            return file_utils::isSupportedVideoFile(filename);

        case DragDropContent::ImagesAndVideos:
            return file_utils::isSupportedImage(filename) ||
                   file_utils::isSupportedVideoFile(filename);
    }

    return false;
}

QString DragDropBehavior::placeholderText() const
{
    return placeholder_;
}

} // namespace fluvel
