// view_behavior.hpp
#pragma once

#include <QtCore/Qt>

#include <QWheelEvent>
#include <QMouseEvent>

namespace ofeli_app
{

class ImageView;

class ViewBehavior
{
public:
    virtual ~ViewBehavior() = default;

    virtual void wheel(ImageView&, QWheelEvent*) {}
    virtual void mousePress(ImageView&, QMouseEvent*) {}
    virtual void mouseMove(ImageView&, QMouseEvent*) {}
    virtual void mouseRelease(ImageView&, QMouseEvent*) {}
    virtual void mouseDoubleClick(ImageView&, QMouseEvent*) {}

    virtual bool isCapturing() const { return false; }

    virtual Qt::CursorShape activeCursor() const
    {
        return Qt::ArrowCursor;
    }

    virtual Qt::CursorShape availableCursor(
        bool /*hasImage*/,
        bool /*isPanRelevant*/,
        const ImageView&,
        const QMouseEvent*) const
    {
        return Qt::ArrowCursor;
    }

    virtual int priority() const { return 0; }
};

} // namespace ofeli_app
