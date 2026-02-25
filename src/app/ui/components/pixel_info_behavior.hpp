#pragma once

#include <memory>

#include <QPoint>

#include "view_behavior.hpp"

class QMouseEvent;

namespace ofeli_app
{

class ImageView;
class PixelInfoOverlay;

/**
 * @brief Passive behavior that displays pixel information
 *        while the right mouse button is held down.
 *
 * - No capture
 * - No cursor change
 * - No event consumption
 * - Optional by construction (not added = disabled)
 */
class PixelInfoBehavior final : public ViewBehavior
{
public:
    explicit PixelInfoBehavior();
    ~PixelInfoBehavior() override;

    Qt::CursorShape activeCursor() const override
    {
        return Qt::CrossCursor;
    }

    bool isCapturing() const override
    {
        return active_;
    }

    bool mousePress(ImageView& view, QMouseEvent* event) override;
    bool mouseMove(ImageView& view, QMouseEvent* event) override;
    bool mouseRelease(ImageView& view, QMouseEvent* event) override;

private:
    void updateOverlay(ImageView& view, const QPoint& viewPos);

private:
    bool active_ = false;

    std::unique_ptr<PixelInfoOverlay> overlay_;
};

} // namespace ofeli_app
