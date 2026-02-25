// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_view.hpp"
#include "view_behavior.hpp"

#include <functional>

#include <QColor>

class QMouseEvent;
class QColor;
class QPoint;

namespace ofeli_app
{

class ImageView;

class ColorPickerBehavior : public ViewBehavior
{
public:
    explicit ColorPickerBehavior(Qt::MouseButton button = Qt::RightButton);

    Qt::CursorShape availableCursor(bool hasImage, bool /*isZoomed*/, const ImageView&,
                                    const QMouseEvent*) const override
    {
        return hasImage ? Qt::CrossCursor : Qt::ArrowCursor;
    }

    int priority() const override
    {
        return 30;
    }

protected:
    bool mousePress(ImageView& view, QMouseEvent* e) override;

private:
    Qt::MouseButton button_;

signals:
    void colorPicked(const QColor& color, const QPoint& imgPos);
};

} // namespace ofeli_app
