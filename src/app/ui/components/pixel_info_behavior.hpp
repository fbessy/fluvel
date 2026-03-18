// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "pixel_info_overlay.hpp"
#include "image_viewer_behavior.hpp"
#include <QPoint>
#include <memory>

class QMouseEvent;

namespace fluvel_app
{

class ImageViewerWidget;

/**
 * @brief Passive behavior that displays pixel information
 *        while the right mouse button is held down.
 *
 * - No capture
 * - No cursor change
 * - No event consumption
 * - Optional by construction (not added = disabled)
 */
class PixelInfoBehavior final : public ImageViewerBehavior
{
public:
    PixelInfoBehavior() = default;
    ~PixelInfoBehavior() override = default;

    bool isActive() const
    {
        return active_;
    }

    Qt::CursorShape activeCursor() const override
    {
        return Qt::CrossCursor;
    }

    bool isCapturing() const override
    {
        return active_;
    }

    bool mousePress(ImageViewerWidget& view, QMouseEvent* event) override;
    bool mouseMove(ImageViewerWidget& view, QMouseEvent* event) override;
    bool mouseRelease(ImageViewerWidget& view, QMouseEvent* event) override;
    void cancel() override;

private:
    void updateOverlay(ImageViewerWidget& view, const QPoint& viewPos);

private:
    bool active_ = false;

    std::unique_ptr<PixelInfoOverlay> overlay_;
};

} // namespace fluvel_app
