// view_behavior.hpp
#pragma once

#include <QtCore/Qt>

#include <QMouseEvent>
#include <QWheelEvent>

namespace ofeli_app
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
};

} // namespace ofeli_app
