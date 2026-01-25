#pragma once

#include "view_behavior.hpp"

class QMouseEvent;

namespace ofeli_app
{

class PanBehavior : public ViewBehavior
{
public:
    explicit PanBehavior(Qt::MouseButton button = Qt::LeftButton);

    void mousePress(ImageView& view, QMouseEvent* event) override;
    void mouseMove(ImageView& view, QMouseEvent* event) override;
    void mouseRelease(ImageView& view, QMouseEvent* event) override;

private:
    Qt::MouseButton button_;
    bool dragging_ = false;
    QPoint lastPos_;
};

} // namespace ofeli_app
