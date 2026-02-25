#include "interaction_set.hpp"

namespace ofeli_app
{

void InteractionSet::addBehavior(std::unique_ptr<ViewBehavior> behavior)
{
    if (behavior)
        behaviors_.push_back(std::move(behavior));
}

bool InteractionSet::wheel(ImageView& view, QWheelEvent* event)
{
    for (auto& b : behaviors_)
    {
        if (b->wheel(view, event))
            return true;
    }

    return false;
}

bool InteractionSet::mousePress(ImageView& view, QMouseEvent* event)
{
    for (auto& b : behaviors_)
    {
        if (b->mousePress(view, event))
            return true;
    }

    return false;
}

bool InteractionSet::mouseMove(ImageView& view, QMouseEvent* event)
{
    for (auto& b : behaviors_)
    {
        if (b->mouseMove(view, event))
            return true;
    }

    return false;
}

bool InteractionSet::mouseRelease(ImageView& view, QMouseEvent* event)
{
    for (auto& b : behaviors_)
    {
        if (b->mouseRelease(view, event))
            return true;
    }

    return false;
}

bool InteractionSet::mouseDoubleClick(ImageView& view, QMouseEvent* event)
{
    for (auto& b : behaviors_)
    {
        if (b->mouseDoubleClick(view, event))
            return true;
    }

    return false;
}

const ViewBehavior* InteractionSet::capturingBehavior() const
{
    for (const auto& b : behaviors_)
    {
        if (b->isCapturing())
            return b.get();
    }
    return nullptr;
}

Qt::CursorShape InteractionSet::cursor(const ImageView& view, bool hasImage, bool isPanRelevant,
                                       const QMouseEvent* e) const
{
    // 1️⃣ Action en cours → priorité absolue
    if (const auto* b = capturingBehavior())
        return b->activeCursor();

    // 2️⃣ Sinon : curseur de disponibilité (arbitrage)
    Qt::CursorShape result = Qt::ArrowCursor;
    int best = std::numeric_limits<int>::min();

    for (const auto& b : behaviors_)
    {
        const auto c = b->availableCursor(hasImage, isPanRelevant, view, e);

        if (c != Qt::ArrowCursor && b->priority() > best)
        {
            best = b->priority();
            result = c;
        }
    }

    return result;
}

Qt::CursorShape InteractionSet::cursorForEvent(const ImageView& view, bool hasImage,
                                               bool isPanRelevant, const QMouseEvent* event) const
{
    return cursor(view, hasImage, isPanRelevant, event);
}

} // namespace ofeli_app
