// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_behavior.hpp"

class QMouseEvent;

namespace fluvel_app
{

class FullscreenBehavior : public ImageViewerBehavior
{
public:
    bool mouseDoubleClick(ImageViewerWidget& view, QMouseEvent* event) override;
};

} // namespace fluvel_app
