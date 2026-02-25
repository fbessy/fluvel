#pragma once

#include "view_behavior.hpp"

class QMouseEvent;

namespace ofeli_app
{

class PanBehavior : public ViewBehavior
{
public:
    explicit PanBehavior(Qt::MouseButton button = Qt::LeftButton);

    bool mousePress(ImageView& view, QMouseEvent* event) override;
    bool mouseMove(ImageView& view, QMouseEvent* event) override;
    bool mouseRelease(ImageView& view, QMouseEvent* event) override;

    bool isCapturing() const override
    {
        return capturing_;
    }

    Qt::CursorShape activeCursor() const override
    {
        return Qt::ClosedHandCursor;
    }

    Qt::CursorShape availableCursor(bool hasImage, bool isPanRelevant, const ImageView&,
                                    const QMouseEvent*) const override
    {
        if (hasImage && isPanRelevant)
            return Qt::OpenHandCursor;

        return Qt::ArrowCursor;
    }

    int priority() const override
    {
        return 10;
    }

private:
    Qt::MouseButton button_;
    bool capturing_ = false;
    QPoint lastPos_;
};

} // namespace ofeli_app
