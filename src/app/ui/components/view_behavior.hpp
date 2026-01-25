// view_behavior.hpp
#pragma once

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
};

} // namespace ofeli_app
