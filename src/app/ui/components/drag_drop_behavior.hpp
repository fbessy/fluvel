// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_widget.hpp"
#include "image_viewer_behavior.hpp"

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QString;

namespace fluvel
{

/**
 * @brief Types of content accepted by drag-and-drop.
 */
enum class DragDropContent
{
    Images,         ///< Accept image files only.
    Videos,         ///< Accept video files only.
    ImagesAndVideos ///< Accept both image and video files.
};

/**
 * @brief Behavior handling drag-and-drop of files.
 *
 * This behavior enables drag-and-drop interactions on an image viewer
 * and filters accepted files according to the configured content type.
 *
 * It manages drag enter/move/leave feedback, placeholder text and
 * drop notifications.
 *
 * Priority:
 * - High priority to ensure drag-and-drop interactions are handled
 *   before other behaviors.
 */
class DragDropBehavior : public ImageViewerBehavior
{
public:
    /**
     * @brief Creates a drag-and-drop behavior.
     *
     * @param content Accepted content type.
     * @param placeholder Placeholder text displayed when no content is loaded.
     */
    explicit DragDropBehavior(DragDropContent content, const QString& placeholder);

    /**
     * @name Drag & drop events
     * @brief Handle drag-and-drop interactions.
     * @{
     */
    bool dragEnter(ImageViewerWidget&, QDragEnterEvent*) override;
    bool dragMove(ImageViewerWidget&, QDragMoveEvent*) override;
    bool dragLeave(ImageViewerWidget&, QDragLeaveEvent*) override;
    bool drop(ImageViewerWidget&, QDropEvent*) override;
    /** @} */

    /**
     * @brief Returns the behavior priority.
     *
     * A high priority ensures drag-and-drop is handled before
     * other interactions.
     */
    int priority() const override
    {
        return 100;
    }

    /**
     * @brief Returns the placeholder text displayed by the viewer.
     */
    QString placeholderText() const override;

private:
    /**
     * @brief Checks whether a file is accepted by this behavior.
     *
     * The validation depends on the configured drag-and-drop content type.
     */
    bool acceptsFile(const QString& filename) const;

    DragDropContent content_; ///< Accepted drag-and-drop content type.

    QString placeholder_; ///< Viewer placeholder text.
};

} // namespace fluvel
