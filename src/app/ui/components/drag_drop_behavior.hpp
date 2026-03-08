// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_view.hpp"
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
    bool dragEnter(ImageView&, QDragEnterEvent*) override;
    bool dragMove(ImageView&, QDragMoveEvent*) override;
    bool dragLeave(ImageView&, QDragLeaveEvent*) override;
    bool drop(ImageView&, QDropEvent*) override;

    int priority() const override
    {
        return 100;
    }
};

} // namespace fluvel_app
