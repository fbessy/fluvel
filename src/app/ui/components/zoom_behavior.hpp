#pragma once

#include "view_behavior.hpp"

namespace ofeli_app {

class ZoomBehavior : public ViewBehavior
{
public:
    explicit ZoomBehavior(double zoomFactor = 1.15);

    bool wheel(ImageView& /*view*/,
               QWheelEvent* event) override;

private:
    //double zoomFactor_;
};

} // namespace ofeli_app
