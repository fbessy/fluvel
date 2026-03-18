// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_interaction.hpp"
#include "image_viewer_behavior.hpp"

#include <QtCore/Qt>

#include <memory>
#include <vector>

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

namespace fluvel_app
{

/**
 * @brief Aggregates multiple ImageViewerBehavior objects.
 *
 * InteractionSet is the concrete ImageViewerInteraction used by ImageViewerWidget.
 * It forwards each event to all registered behaviors.
 */
class InteractionSet : public ImageViewerInteraction
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

    void addBehavior(std::unique_ptr<ImageViewerBehavior> behavior);

    const ImageViewerBehavior* capturingBehavior() const;

    Qt::CursorShape cursor(const ImageViewerWidget& view, bool hasImage, bool isPanRelevant,
                           const QMouseEvent* e) const;

    template <typename T> bool hasBehavior() const
    {
        for (const auto& b : behaviors_)
        {
            if (dynamic_cast<const T*>(b.get()))
                return true;
        }
        return false;
    }

protected:
    bool wheel(ImageViewerWidget& view, QWheelEvent* event) override;
    bool mousePress(ImageViewerWidget& view, QMouseEvent* event) override;
    bool mouseMove(ImageViewerWidget& view, QMouseEvent* event) override;
    bool mouseRelease(ImageViewerWidget& view, QMouseEvent* event) override;
    bool mouseDoubleClick(ImageViewerWidget& view, QMouseEvent* event) override;

    Qt::CursorShape cursorForEvent(const ImageViewerWidget& view, bool hasImage, bool isPanRelevant,
                                   const QMouseEvent* event) const override;

    bool dragEnter(ImageViewerWidget& view, QDragEnterEvent* event) override;
    bool dragMove(ImageViewerWidget& view, QDragMoveEvent* event) override;
    bool dragLeave(ImageViewerWidget& view, QDragLeaveEvent* event) override;
    bool drop(ImageViewerWidget& view, QDropEvent* event) override;

    void cancel() override;

private:
    std::vector<std::unique_ptr<ImageViewerBehavior>> behaviors_;
};

} // namespace fluvel_app
