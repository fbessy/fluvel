// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QMouseEvent>
#include <QWheelEvent>

#include <QtCore/Qt>

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

namespace fluvel_app
{

class ImageViewerWidget;

class ImageViewerInteraction
{
public:
    virtual ~ImageViewerInteraction() = default;

    virtual bool wheel(ImageViewerWidget&, QWheelEvent*)
    {
        return false;
    }
    virtual bool mousePress(ImageViewerWidget&, QMouseEvent*)
    {
        return false;
    }
    virtual bool mouseMove(ImageViewerWidget&, QMouseEvent*)
    {
        return false;
    }
    virtual bool mouseRelease(ImageViewerWidget&, QMouseEvent*)
    {
        return false;
    }
    virtual bool mouseDoubleClick(ImageViewerWidget&, QMouseEvent*)
    {
        return false;
    }

    virtual Qt::CursorShape cursorForEvent(const ImageViewerWidget& /*view*/, bool /*hasImage*/,
                                           bool /*isPanRelevant*/,
                                           const QMouseEvent* /*event*/) const
    {
        return Qt::ArrowCursor;
    }

    virtual bool dragEnter(ImageViewerWidget&, QDragEnterEvent*)
    {
        return false;
    }

    virtual bool dragMove(ImageViewerWidget&, QDragMoveEvent*)
    {
        return false;
    }

    virtual bool dragLeave(ImageViewerWidget&, QDragLeaveEvent*)
    {
        return false;
    }

    virtual bool drop(ImageViewerWidget&, QDropEvent*)
    {
        return false;
    }

    virtual void cancel()
    {
    }
};

} // namespace fluvel_app
