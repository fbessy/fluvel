// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "interaction_set.hpp"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

namespace fluvel_app
{

void InteractionSet::addBehavior(std::unique_ptr<ViewBehavior> behavior)
{
    if (!behavior)
        return;

    behaviors_.push_back(std::move(behavior));

    std::sort(behaviors_.begin(), behaviors_.end(),
              [](const auto& a, const auto& b)
              {
                  return a->priority() > b->priority();
              });
}

bool InteractionSet::wheel(ImageViewerWidget& view, QWheelEvent* event)
{
    for (auto& b : behaviors_)
    {
        if (b->wheel(view, event))
            return true;
    }

    return false;
}

bool InteractionSet::mousePress(ImageViewerWidget& view, QMouseEvent* event)
{
    if (auto* b = capturingBehavior())
        return const_cast<ViewBehavior*>(b)->mousePress(view, event);

    for (auto& b : behaviors_)
    {
        if (b->mousePress(view, event))
            return true;
    }

    return false;
}

bool InteractionSet::mouseMove(ImageViewerWidget& view, QMouseEvent* event)
{
    if (auto* b = capturingBehavior())
        return const_cast<ViewBehavior*>(b)->mouseMove(view, event);

    for (auto& b : behaviors_)
    {
        if (b->mouseMove(view, event))
            return true;
    }

    return false;
}

bool InteractionSet::mouseRelease(ImageViewerWidget& view, QMouseEvent* event)
{
    if (auto* b = capturingBehavior())
        return const_cast<ViewBehavior*>(b)->mouseRelease(view, event);

    for (auto& b : behaviors_)
    {
        if (b->mouseRelease(view, event))
            return true;
    }

    return false;
}

bool InteractionSet::mouseDoubleClick(ImageViewerWidget& view, QMouseEvent* event)
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

Qt::CursorShape InteractionSet::cursor(const ImageViewerWidget& view, bool hasImage, bool isPanRelevant,
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

Qt::CursorShape InteractionSet::cursorForEvent(const ImageViewerWidget& view, bool hasImage,
                                               bool isPanRelevant, const QMouseEvent* event) const
{
    return cursor(view, hasImage, isPanRelevant, event);
}

bool InteractionSet::dragEnter(ImageViewerWidget& view, QDragEnterEvent* event)
{
    for (auto& behavior : behaviors_)
    {
        if (behavior->dragEnter(view, event))
            return true;
    }
    return false;
}

bool InteractionSet::dragMove(ImageViewerWidget& view, QDragMoveEvent* event)
{
    for (auto& behavior : behaviors_)
    {
        if (behavior->dragMove(view, event))
            return true;
    }
    return false;
}

bool InteractionSet::dragLeave(ImageViewerWidget& view, QDragLeaveEvent* event)
{
    for (auto& behavior : behaviors_)
    {
        if (behavior->dragLeave(view, event))
            return true;
    }
    return false;
}

bool InteractionSet::drop(ImageViewerWidget& view, QDropEvent* event)
{
    for (auto& behavior : behaviors_)
    {
        if (behavior->drop(view, event))
            return true;
    }
    return false;
}

void InteractionSet::cancel()
{
    if (auto* b = capturingBehavior())
        const_cast<ViewBehavior*>(b)->cancel();
}

} // namespace fluvel_app
