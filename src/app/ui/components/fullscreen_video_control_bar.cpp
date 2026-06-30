// SPDX-License-Identifier: CeCILL-2.1

#include "fullscreen_video_control_bar.hpp"

#include "clickable_label.hpp"
#include "icon_loader.hpp"
#include "timeline_slider.hpp"
#include "ui_theme.hpp"
#include "volume_controller.hpp"
#include "volume_slider.hpp"

#include <QColor>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>

namespace fluvel
{

FullscreenVideoControlBar::FullscreenVideoControlBar(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);

    auto* background = new QWidget(this);
    background->setObjectName("fullscreenBackground");

    background->setStyleSheet(R"(
#fullscreenBackground {
    background-color: rgba(32,36,42,210);
    border-radius: 16px;
}
)");

    setStyleSheet(R"(
QLabel {
    color: white;
}
QComboBox {
    min-width: 220px;
}

QLabel:disabled {
    color: rgba(255,255,255,70);
}

QComboBox {
    color: white;
    background-color: rgba(255,255,255,20);
    border: 1px solid rgba(255,255,255,35);
    border-radius: 15px;
    padding: 4px 28px 4px 10px;
    min-height: 30px;
}

QComboBox:hover {
    background-color: rgba(255,255,255,30);
}

QComboBox:focus {
    border: 1px solid #8B5CF6;
}

QComboBox::drop-down {
    border: none;
    width: 24px;
}

QComboBox::down-arrow {
    image: none;
}

QComboBox QAbstractItemView {
    background: rgb(32,36,42);
    color: white;
    border: 1px solid rgb(60,60,60);
    border-radius: 12px;
    selection-background-color: rgba(255,255,255,25);
    outline: 0;
}

QComboBox QAbstractItemView::viewport {
    background: rgb(32,36,42);
}


QComboBox QAbstractItemView::item:selected {
    background-color: rgba(255,255,255,25);
}

QComboBox QAbstractItemView::item:hover {
    background-color: rgba(255,255,255,15);
}

QScrollBar:vertical {
    width: 0px;
}

QScrollBar:horizontal {
    height: 0px;
}
)");

    cameraSelector_ = new QComboBox(this);
    cameraSelector_->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    auto* view = new QListView(cameraSelector_);
    view->setFrameShape(QFrame::NoFrame);
    view->viewport()->setAutoFillBackground(false);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    cameraSelector_->setView(view);

    startStopButton_ = new StyledToolButton(this, ui::Appearance::Modern);
    playPauseButton_ = new StyledToolButton(this, ui::Appearance::Modern);
    volumeController_ = new VolumeController(this, ui::Appearance::Modern);

    mirrorButton_ = new StyledToolButton(this, ui::Appearance::Modern);
    smoothButton_ = new StyledToolButton(this, ui::Appearance::Modern);
    overlayButton_ = new StyledToolButton(this, ui::Appearance::Modern);

    mirrorButton_->setCheckable(true);
    smoothButton_->setCheckable(true);
    overlayButton_->setCheckable(true);

    mirrorButton_->setIcon(il::loadIcon(":/icons/view/mirror-symbolic.svg", il::IconMode::Light));
    smoothButton_->setIcon(
        il::loadIcon(":/icons/actions/smooth_rendering-symbolic.svg", il::IconMode::Light));
    overlayButton_->setIcon(
        il::loadIcon(":/icons/actions/help-about-symbolic.svg", il::IconMode::Light));

    constexpr QSize kIconSize(ui::kButtonIconSize, ui::kButtonIconSize);
    constexpr QSize kButtonSize(ui::kButtonSize, ui::kButtonSize);

    for (auto* b : {startStopButton_, playPauseButton_, volumeController_->button(), mirrorButton_,
                    smoothButton_, overlayButton_})
    {
        b->setIconSize(kIconSize);
        b->setFixedSize(kButtonSize);
        b->setCursor(Qt::PointingHandCursor);
    }

    positionLabel_ = new QLabel("00:00");
    durationLabel_ = new ClickableLabel("-00:00");

    QFont font = positionLabel_->font();
    font.setBold(true);
    font.setPointSize(14);

    positionLabel_->setFont(font);
    durationLabel_->setFont(font);

    playbackSlider_ = new TimelineSlider(this, ui::Appearance::Modern);
    playbackSlider_->setMinimumWidth(260);
    playbackSlider_->setMaximumWidth(520);
    playbackSlider_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto* transportLayout = new QHBoxLayout;
    transportLayout->setContentsMargins(0, 0, 0, 0);
    transportLayout->setSpacing(6);

    transportLayout->addWidget(cameraSelector_);
    transportLayout->addWidget(startStopButton_);
    transportLayout->addWidget(volumeController_->button());
    transportLayout->addWidget(playPauseButton_);

    auto* mediaLayout = new QVBoxLayout;
    mediaLayout->setContentsMargins(0, 0, 0, 0);
    mediaLayout->setSpacing(2);

    mediaLayout->addWidget(volumeController_->slider());
    mediaLayout->addLayout(transportLayout);

    auto* topLayout = new QHBoxLayout;
    topLayout->setSpacing(6);

    topLayout->addWidget(cameraSelector_);
    topLayout->addWidget(startStopButton_);

    topLayout->addSpacing(8);

    topLayout->addWidget(playPauseButton_);

    topLayout->addSpacing(8);

    topLayout->addWidget(volumeController_->button());
    topLayout->addWidget(volumeController_->slider());

    topLayout->addWidget(mirrorButton_);
    topLayout->addWidget(smoothButton_);
    topLayout->addWidget(overlayButton_);

    auto* bottomLayout = new QHBoxLayout;
    bottomLayout->setSpacing(6);

    bottomLayout->addWidget(positionLabel_);
    bottomLayout->addWidget(playbackSlider_, 1);
    bottomLayout->addWidget(durationLabel_);

    auto* layout = new QVBoxLayout;
    layout->setContentsMargins(18, 10, 18, 10);
    layout->setSpacing(0);

    layout->addLayout(topLayout);
    layout->addLayout(bottomLayout);

    background->setLayout(layout);

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(background, 0, Qt::AlignCenter);
}

QComboBox* FullscreenVideoControlBar::cameraSelector() const
{
    return cameraSelector_;
}

StyledToolButton* FullscreenVideoControlBar::startStopButton() const
{
    return startStopButton_;
}

VolumeController* FullscreenVideoControlBar::volumeController() const
{
    return volumeController_;
}

StyledToolButton* FullscreenVideoControlBar::playPauseButton() const
{
    return playPauseButton_;
}

QLabel* FullscreenVideoControlBar::positionLabel() const
{
    return positionLabel_;
}

TimelineSlider* FullscreenVideoControlBar::playbackSlider() const
{
    return playbackSlider_;
}

ClickableLabel* FullscreenVideoControlBar::durationLabel() const
{
    return durationLabel_;
}

StyledToolButton* FullscreenVideoControlBar::mirrorButton() const
{
    return mirrorButton_;
}

StyledToolButton* FullscreenVideoControlBar::smoothButton() const
{
    return smoothButton_;
}

StyledToolButton* FullscreenVideoControlBar::overlayButton() const
{
    return overlayButton_;
}

} // namespace fluvel
