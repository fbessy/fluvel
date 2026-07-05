// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "autofit_behavior.hpp"
#include "image_viewer_widget.hpp"

#include <QMouseEvent>
#include <QObject>

namespace fluvel
{

AutoFitBehavior::AutoFitBehavior(Qt::MouseButton button)
    : button_(button)
{
}

bool AutoFitBehavior::mouseRelease(ImageViewerWidget& view, QMouseEvent* event)
{
    if (event->button() != button_)
        return false;

    view.applyAutoFit();
    view.showCursorMessage(QObject::tr("Auto Fit"));

    event->accept();
    return true;
}

} // namespace fluvel
