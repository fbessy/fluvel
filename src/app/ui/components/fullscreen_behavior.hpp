#ifndef FULLSCREEN_BEHAVIOR_HPP
#define FULLSCREEN_BEHAVIOR_HPP

#include "view_behavior.hpp"

class QMouseEvent;

namespace ofeli_app
{

class FullscreenBehavior : public ViewBehavior
{
public:
    bool mouseDoubleClick(ImageView& view,
                          QMouseEvent* event) override;
};

}
#endif // FULLSCREEN_BEHAVIOR_HPP
