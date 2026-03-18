// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

// view_behavior.hpp
#pragma once

#include <QWheelEvent>
#include <QtCore/Qt>

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

class QMouseEvent;

namespace fluvel_app
{

class ImageViewerWidget;

class ViewBehavior
{
public:
    virtual ~ViewBehavior() = default;

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

    virtual bool isCapturing() const
    {
        return false;
    }

    virtual Qt::CursorShape activeCursor() const
    {
        return Qt::ArrowCursor;
    }

    virtual Qt::CursorShape availableCursor(bool /*hasImage*/, bool /*isPanRelevant*/,
                                            const ImageViewerWidget&, const QMouseEvent*) const
    {
        return Qt::ArrowCursor;
    }

    virtual int priority() const
    {
        return 0;
    }

    virtual void cancel() {};
};

} // namespace fluvel_app
