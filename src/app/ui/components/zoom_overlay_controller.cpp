// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "zoom_overlay_controller.hpp"
#include "overlay_text_item.hpp"

#include <QGraphicsItem>
#include <QPropertyAnimation>

namespace fluvel_app
{

ZoomOverlayController::ZoomOverlayController(OverlayTextItem* item, QObject* parent)
    : QObject(parent)
    , item_(item)
{
    timer_.setSingleShot(true);

    connect(&timer_, &QTimer::timeout, this, &ZoomOverlayController::onTimeout);

    item_->setOpacity(1.0);
    item_->setVisible(false);
}

void ZoomOverlayController::show(int percent)
{
    if (!item_)
        return;

    // stop animation en cours
    if (anim_)
    {
        anim_->stop();
        anim_->deleteLater();
        anim_ = nullptr;
    }

    item_->setOpacity(1.0);

    // texte
    item_->setText(QString("%1%").arg(percent));

    // centrage
    QRectF bounds = item_->boundingRect();

    item_->setVisible(true);

    // restart timer
    timer_.start(displayDurationMs_);
}

void ZoomOverlayController::onTimeout()
{
    startFade();
}

void ZoomOverlayController::startFade()
{
    if (!item_)
        return;

    anim_ = new QPropertyAnimation(item_, "opacity", this);
    anim_->setDuration(fadeDurationMs_);
    anim_->setStartValue(1.0);
    anim_->setEndValue(0.0);

    connect(anim_, &QPropertyAnimation::finished, this, &ZoomOverlayController::onFadeFinished);

    anim_->start(QAbstractAnimation::DeleteWhenStopped);
}

void ZoomOverlayController::onFadeFinished()
{
    if (!item_)
        return;

    item_->setVisible(false);
    item_->setOpacity(1.0);

    anim_ = nullptr;
}

} // namespace fluvel_app
