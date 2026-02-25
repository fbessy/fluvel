#ifndef AUTOFIT_BEHAVIOR_HPP
#define AUTOFIT_BEHAVIOR_HPP

#include "view_behavior.hpp"

class QMouseEvent;

namespace ofeli_app
{

class AutoFitBehavior : public ViewBehavior
{
public:
    explicit AutoFitBehavior(Qt::MouseButton button = Qt::MiddleButton);

    bool mouseRelease(ImageView& view, QMouseEvent* event) override;

private:
    Qt::MouseButton button_;
};

} // namespace ofeli_app

#endif // AUTOFIT_BEHAVIOR_HPP
