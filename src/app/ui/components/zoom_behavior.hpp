#pragma once

#include "view_behavior.hpp"

namespace ofeli_app {

class ZoomBehavior : public ViewBehavior
{
public:
    explicit ZoomBehavior(double minZoom = 0.1,
                          double maxZoom = 20.0,
                          double zoomFactor = 1.15);

    void wheel(ImageView& view,
               QWheelEvent* event) override;

private:
    double minZoom_;
    double maxZoom_;
    double zoomFactor_;
};

} // namespace ofeli_app
