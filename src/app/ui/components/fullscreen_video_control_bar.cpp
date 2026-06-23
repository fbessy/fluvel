// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "fullscreen_video_control_bar.hpp"

#include "timeline_slider.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

namespace fluvel
{

FullscreenVideoControlBar::FullscreenVideoControlBar(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);

    auto* backgroundWidget = new QWidget(this);

    backgroundWidget->setObjectName("fullscreenBackground");

    backgroundWidget->setStyleSheet(R"(
#fullscreenBackground
{
    background-color: rgba(0,0,0,180);
    border-radius: 8px;
}
)");

    setStyleSheet(R"(
QPushButton
{
    background: transparent;

    border: none;

    padding: 10px;

    min-width: 56px;
    min-height: 56px;
}

QPushButton:hover
{
    background-color: rgba(255,255,255,40);

    border-radius: 6px;
}

QPushButton:pressed
{
    background-color: rgba(255,255,255,80);

    border-radius: 6px;
}

QLabel
{
    color: white;
}

QPushButton:disabled
{
    opacity: 0.5;
}
)");

    startStopButton_ = new QPushButton;
    playPauseButton_ = new QPushButton;
    volumeButton_ = new QPushButton;

    positionLabel_ = new QLabel("00:00");
    durationLabel_ = new QLabel("00:00");

    QFont font = positionLabel_->font();

    font.setBold(true);
    font.setPointSize(14);

    positionLabel_->setFont(font);
    durationLabel_->setFont(font);

    playbackSlider_ = new TimelineSlider(this, true);
    playbackSlider_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    playbackSlider_->setFixedHeight(80);

    qDebug() << playbackSlider_->size();

    constexpr QSize kIconSize(32, 32);

    startStopButton_->setIconSize(kIconSize);
    playPauseButton_->setIconSize(kIconSize);
    volumeButton_->setIconSize(kIconSize);

    startStopButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    playPauseButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    volumeButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto* controlsLayout = new QHBoxLayout;

    controlsLayout->setContentsMargins(12, 12, 12, 12);
    controlsLayout->setSpacing(12);

    controlsLayout->addWidget(startStopButton_);
    controlsLayout->addWidget(playPauseButton_);

    controlsLayout->addWidget(positionLabel_);

    controlsLayout->addWidget(playbackSlider_, 1);

    controlsLayout->addWidget(durationLabel_);

    // to do
    controlsLayout->addWidget(volumeButton_);

    controlsLayout->setAlignment(Qt::AlignLeft);

    backgroundWidget->setLayout(controlsLayout);

    auto* rootLayout = new QHBoxLayout(this);

    rootLayout->setContentsMargins(0, 0, 0, 0);

    rootLayout->addWidget(backgroundWidget);

    setLayout(rootLayout);
}

QPushButton* FullscreenVideoControlBar::startStopButton() const
{
    return startStopButton_;
}

QPushButton* FullscreenVideoControlBar::playPauseButton() const
{
    return playPauseButton_;
}

TimelineSlider* FullscreenVideoControlBar::playbackSlider() const
{
    return playbackSlider_;
}

QLabel* FullscreenVideoControlBar::positionLabel() const
{
    return positionLabel_;
}

QLabel* FullscreenVideoControlBar::durationLabel() const
{
    return durationLabel_;
}

QPushButton* FullscreenVideoControlBar::volumeButton() const
{
    return volumeButton_;
}

} // namespace fluvel