// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "autofit_behavior.hpp"
#include "image_view.hpp"

#include <QMouseEvent>

namespace fluvel_app
{

AutoFitBehavior::AutoFitBehavior(Qt::MouseButton button)
    : button_(button)
{
}

bool AutoFitBehavior::mouseRelease(ImageView& view, QMouseEvent* event)
{
    if (event->button() == button_)
    {
        view.applyAutoFit();
        event->accept();
        return true;
    }

    return false;
}

} // namespace fluvel_app
