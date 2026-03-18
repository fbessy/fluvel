// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_widget.hpp"
#include "view_behavior.hpp"

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

namespace fluvel_app
{

class DragDropBehavior : public ViewBehavior
{
public:
    bool dragEnter(ImageViewerWidget&, QDragEnterEvent*) override;
    bool dragMove(ImageViewerWidget&, QDragMoveEvent*) override;
    bool dragLeave(ImageViewerWidget&, QDragLeaveEvent*) override;
    bool drop(ImageViewerWidget&, QDropEvent*) override;

    int priority() const override
    {
        return 100;
    }
};

} // namespace fluvel_app
