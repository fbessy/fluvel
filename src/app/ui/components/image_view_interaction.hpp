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

class ImageView;

class ImageViewInteraction
{
public:
    virtual ~ImageViewInteraction() = default;

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

    virtual Qt::CursorShape cursorForEvent(const ImageView& /*view*/, bool /*hasImage*/,
                                           bool /*isPanRelevant*/,
                                           const QMouseEvent* /*event*/) const
    {
        return Qt::ArrowCursor;
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

    virtual void cancel()
    {
    }
};

} // namespace fluvel_app
