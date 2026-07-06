// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "hud_overlay_controller.hpp"
#include "overlay_text_item.hpp"

#include <QGraphicsItem>
#include <QPropertyAnimation>

namespace fluvel
{

HudOverlayController::HudOverlayController(OverlayTextItem* item, HudPreset preset, QObject* parent)
    : QObject(parent)
    , item_(item)
{
    assert(item_);

    applyPreset(preset);

    timer_.setSingleShot(true);

    connect(&timer_, &QTimer::timeout, this, &HudOverlayController::onTimeout);

    anim_.setTargetObject(item_);
    anim_.setPropertyName("opacity");

    connect(&anim_, &QPropertyAnimation::finished, this, &HudOverlayController::onFadeFinished);

    item_->setOpacity(1.0);
    item_->setVisible(false);
}

void HudOverlayController::show(const QString& text)
{
    anim_.stop();

    item_->setOpacity(1.0);
    item_->setText(text);
    item_->setVisible(true);

    timer_.start(displayDurationMs_);
}

void HudOverlayController::onTimeout()
{
    startFade();
}

void HudOverlayController::startFade()
{
    anim_.stop();

    anim_.setDuration(fadeDurationMs_);
    anim_.setStartValue(1.0);
    anim_.setEndValue(0.0);

    anim_.start();
}

void HudOverlayController::onFadeFinished()
{
    item_->setVisible(false);
    item_->setOpacity(1.0);
}

void HudOverlayController::applyPreset(HudPreset preset)
{
    QFont font = item_->font();

    switch (preset)
    {
        case HudPreset::Cursor:

            font.setPointSize(13);
            font.setBold(true);

            item_->setFont(font);

            displayDurationMs_ = 500;
            fadeDurationMs_ = 150;

            break;

        case HudPreset::Notification:

            font.setPointSize(22);
            font.setBold(true);

            item_->setFont(font);

            displayDurationMs_ = 900;
            fadeDurationMs_ = 180;

            break;
    }
}

} // namespace fluvel
