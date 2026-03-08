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

class ImageView;

class ViewBehavior
{
public:
    virtual ~ViewBehavior() = default;

    virtual bool wheel(ImageView&, QWheelEvent*)
    {
        return false;
    }
    virtual bool mousePress(ImageView&, QMouseEvent*)
    {
        return false;
    }
    virtual bool mouseMove(ImageView&, QMouseEvent*)
    {
        return false;
    }
    virtual bool mouseRelease(ImageView&, QMouseEvent*)
    {
        return false;
    }
    virtual bool mouseDoubleClick(ImageView&, QMouseEvent*)
    {
        return false;
    }

    virtual bool dragEnter(ImageView&, QDragEnterEvent*)
    {
        return false;
    }

    virtual bool dragMove(ImageView&, QDragMoveEvent*)
    {
        return false;
    }

    virtual bool dragLeave(ImageView&, QDragLeaveEvent*)
    {
        return false;
    }

    virtual bool drop(ImageView&, QDropEvent*)
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
                                            const ImageView&, const QMouseEvent*) const
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
