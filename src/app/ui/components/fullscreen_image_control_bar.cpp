// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "fullscreen_image_control_bar.hpp"
#include "animated_push_button.hpp"

#include <QHBoxLayout>

namespace fluvel
{

FullscreenImageControlBar::FullscreenImageControlBar(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);

    setStyleSheet(R"(
QPushButton
{
    background-color: rgba(0,0,0,180);

    border: 1px solid rgba(255,255,255,40);

    border-radius: 8px;

    padding: 10px;

    min-width: 56px;
    min-height: 56px;
}

QPushButton:hover
{
    background-color: rgba(0,0,0,210);
    border: 1px solid rgba(255,255,255,80);

    border-radius: 8px;
}

QPushButton:pressed
{
    background-color: rgba(0,0,0,240);
    border: 1px solid rgba(255,255,255,120);

    border-radius: 8px;
}

QPushButton:disabled
{
    background-color: rgba(0,0,0,100);
    border: 1px solid rgba(255,255,255,25);
}
)");

    restartButton_ = new AnimatedPushButton;

    pauseButton_ = new AnimatedPushButton;
    pauseButton_->setTransitionEffect(TransitionEffect::Flip);
    pauseButton_->setClickAnimation(ClickAnimation::None);

    stepButton_ = new AnimatedPushButton;
    stepButton_->setAutoRepeat(true);
    stepButton_->setAutoRepeatDelay(300);
    stepButton_->setAutoRepeatInterval(100);

    convergeButton_ = new AnimatedPushButton;

    constexpr QSize kIconSize(32, 32);

    restartButton_->setIconSize(kIconSize);
    pauseButton_->setIconSize(kIconSize);
    stepButton_->setIconSize(kIconSize);
    convergeButton_->setIconSize(kIconSize);

    auto* layout = new QHBoxLayout(this);

    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(12);

    layout->addWidget(restartButton_);
    layout->addWidget(pauseButton_);
    layout->addWidget(stepButton_);
    layout->addWidget(convergeButton_);

    setLayout(layout);
}

AnimatedPushButton* FullscreenImageControlBar::restartButton() const
{
    return restartButton_;
}

AnimatedPushButton* FullscreenImageControlBar::pauseButton() const
{
    return pauseButton_;
}

AnimatedPushButton* FullscreenImageControlBar::stepButton() const
{
    return stepButton_;
}

AnimatedPushButton* FullscreenImageControlBar::convergeButton() const
{
    return convergeButton_;
}

} // namespace fluvel