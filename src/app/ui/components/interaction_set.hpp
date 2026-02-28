// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_view_interaction.hpp"
#include "view_behavior.hpp"

#include <QtCore/Qt>

#include <memory>
#include <vector>

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

namespace ofeli_app
{

/**
 * @brief Aggregates multiple ViewBehavior objects.
 *
 * InteractionSet is the concrete ImageViewInteraction used by ImageView.
 * It forwards each event to all registered behaviors.
 */
class InteractionSet : public ImageViewInteraction
{
public:
    InteractionSet() = default;
    ~InteractionSet() override = default;

    // Non copyable (ownership of behaviors)
    InteractionSet(const InteractionSet&) = delete;
    InteractionSet& operator=(const InteractionSet&) = delete;

    // Movable if needed
    InteractionSet(InteractionSet&&) = default;
    InteractionSet& operator=(InteractionSet&&) = default;

    void addBehavior(std::unique_ptr<ViewBehavior> behavior);

    const ViewBehavior* capturingBehavior() const;

    Qt::CursorShape cursor(const ImageView& view, bool hasImage, bool isPanRelevant,
                           const QMouseEvent* e) const;

protected:
    bool wheel(ImageView& view, QWheelEvent* event) override;
    bool mousePress(ImageView& view, QMouseEvent* event) override;
    bool mouseMove(ImageView& view, QMouseEvent* event) override;
    bool mouseRelease(ImageView& view, QMouseEvent* event) override;
    bool mouseDoubleClick(ImageView& view, QMouseEvent* event) override;

    Qt::CursorShape cursorForEvent(const ImageView& view, bool hasImage, bool isPanRelevant,
                                   const QMouseEvent* event) const override;

    bool dragEnter(ImageView& view, QDragEnterEvent* event) override;
    bool dragMove(ImageView& view, QDragMoveEvent* event) override;
    bool dragLeave(ImageView& view, QDragLeaveEvent* event) override;
    bool drop(ImageView& view, QDropEvent* event) override;

private:
    std::vector<std::unique_ptr<ViewBehavior>> behaviors_;
};

} // namespace ofeli_app
