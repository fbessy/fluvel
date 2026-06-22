// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "fullscreen_image_control_bar.hpp"

#include <QHBoxLayout>
#include <QPushButton>

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

    border: 1px solid rgba(255,255,255,130);

    border-radius: 5px;

    padding: 10px;
    min-width: 56px;
    min-height: 56px;
}

QPushButton:hover
{
    background-color: rgba(0,0,0,220);

    border: 1px solid rgba(255,255,255,200);
}

QPushButton:pressed
{
    background-color: rgba(255,255,255,180);

    color: black;
}

QPushButton:disabled
{
    color: rgba(255,255,255,100);
    border: 1px solid rgba(255,255,255,50);
}
)");

    restartButton_ = new QPushButton;
    pauseButton_ = new QPushButton;

    stepButton_ = new QPushButton;
    stepButton_->setAutoRepeat(true);
    stepButton_->setAutoRepeatDelay(300);
    stepButton_->setAutoRepeatInterval(100);

    convergeButton_ = new QPushButton;

    constexpr QSize kIconSize(32, 32);

    restartButton_->setIconSize(kIconSize);
    pauseButton_->setIconSize(kIconSize);
    stepButton_->setIconSize(kIconSize);
    convergeButton_->setIconSize(kIconSize);

    pauseButton_->setStyleSheet("QPushButton { color: white; }");

    auto* layout = new QHBoxLayout(this);

    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(12);

    layout->addWidget(restartButton_);
    layout->addWidget(pauseButton_);
    layout->addWidget(stepButton_);
    layout->addWidget(convergeButton_);

    setLayout(layout);
}

QPushButton* FullscreenImageControlBar::restartButton() const
{
    return restartButton_;
}

QPushButton* FullscreenImageControlBar::pauseButton() const
{
    return pauseButton_;
}

QPushButton* FullscreenImageControlBar::stepButton() const
{
    return stepButton_;
}

QPushButton* FullscreenImageControlBar::convergeButton() const
{
    return convergeButton_;
}

} // namespace fluvel