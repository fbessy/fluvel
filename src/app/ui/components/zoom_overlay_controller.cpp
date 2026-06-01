// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "zoom_overlay_controller.hpp"
#include "overlay_text_item.hpp"

#include <QGraphicsItem>
#include <QPropertyAnimation>

namespace fluvel
{

ZoomOverlayController::ZoomOverlayController(OverlayTextItem* item, QObject* parent)
    : QObject(parent)
    , item_(item)
{
    assert(item_);

    timer_.setSingleShot(true);

    connect(&timer_, &QTimer::timeout, this, &ZoomOverlayController::onTimeout);

    anim_.setTargetObject(item_);
    anim_.setPropertyName("opacity");

    connect(&anim_, &QPropertyAnimation::finished, this, &ZoomOverlayController::onFadeFinished);

    item_->setOpacity(1.0);
    item_->setVisible(false);
}

void ZoomOverlayController::show(int percent)
{
    anim_.stop();

    item_->setOpacity(1.0);
    item_->setText(QString("%1%").arg(percent));
    item_->setVisible(true);

    timer_.start(displayDurationMs_);
}

void ZoomOverlayController::onTimeout()
{
    startFade();
}

void ZoomOverlayController::startFade()
{
    anim_.stop();

    anim_.setDuration(fadeDurationMs_);
    anim_.setStartValue(1.0);
    anim_.setEndValue(0.0);

    anim_.start();
}

void ZoomOverlayController::onFadeFinished()
{
    item_->setVisible(false);
    item_->setOpacity(1.0);
}

} // namespace fluvel
