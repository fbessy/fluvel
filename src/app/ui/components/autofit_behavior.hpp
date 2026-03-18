// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_behavior.hpp"

class QMouseEvent;

namespace fluvel_app
{

class AutoFitBehavior : public ImageViewerBehavior
{
public:
    explicit AutoFitBehavior(Qt::MouseButton button = Qt::MiddleButton);

    bool mouseRelease(ImageViewerWidget& view, QMouseEvent* event) override;

private:
    Qt::MouseButton button_;
};

} // namespace fluvel_app
