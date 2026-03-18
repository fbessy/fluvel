// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_behavior.hpp"

namespace fluvel_app
{

class ZoomBehavior : public ImageViewerBehavior
{
public:
    explicit ZoomBehavior(double zoomFactor = 1.15);

    bool wheel(ImageViewerWidget& /*view*/, QWheelEvent* event) override;

private:
    // double zoomFactor_;
};

} // namespace fluvel_app
