#include "interaction_set.hpp"

namespace ofeli_app {

void InteractionSet::addBehavior(std::unique_ptr<ViewBehavior> behavior)
{
    if (behavior)
        behaviors_.push_back(std::move(behavior));
}

void InteractionSet::wheel(ImageView& view,
                           QWheelEvent* event)
{
    for (auto& b : behaviors_)
    {
        b->wheel(view, event);
        if (event->isAccepted())
            return;
    }
}

void InteractionSet::mousePress(ImageView& view,
                                QMouseEvent* event)
{
    for (auto& b : behaviors_)
        b->mousePress(view, event);
}

void InteractionSet::mouseMove(ImageView& view,
                               QMouseEvent* event)
{
    for (auto& b : behaviors_)
    {
        b->mouseMove(view, event);
    }
}

void InteractionSet::mouseRelease(ImageView& view,
                                  QMouseEvent* event)
{
    for (auto& b : behaviors_)
        b->mouseRelease(view, event);
}

void InteractionSet::mouseDoubleClick(ImageView& view,
                                      QMouseEvent* event)
{
    for (auto& b : behaviors_)
        b->mouseDoubleClick(view, event);
}

} // namespace ofeli_app
