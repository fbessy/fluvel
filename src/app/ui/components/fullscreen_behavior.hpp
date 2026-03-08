// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "view_behavior.hpp"

class QMouseEvent;

namespace fluvel_app
{

class FullscreenBehavior : public ViewBehavior
{
public:
    bool mouseDoubleClick(ImageView& view, QMouseEvent* event) override;
};

} // namespace fluvel_app
