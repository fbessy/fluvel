// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_view.hpp"
#include "view_behavior.hpp"

#include <QPoint>

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

    Qt::CursorShape availableCursor(bool hasImage, bool isPanRelevant, const ImageView& view,
                                    const QMouseEvent* e) const override
    {
        if (!hasImage || !isPanRelevant)
            return Qt::ArrowCursor;

        QPoint pos = e ? e->pos() : view.mapFromGlobal(QCursor::pos());

        const auto items = view.items(pos);

        for (auto* item : items)
        {
            if (item->flags() & QGraphicsItem::ItemIsMovable)
                return Qt::ArrowCursor;
        }

        return Qt::OpenHandCursor;
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
