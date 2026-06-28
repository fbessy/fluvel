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

    cameraSelector_ = new QComboBox;

    auto* view = new QListView(cameraSelector_);
    view->setFrameShape(QFrame::NoFrame);
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

    QWidget* widgets[] = {startStopButton_,
                          playPauseButton_,
                          volumeController_->button(),
                          volumeController_->slider(),
                          positionLabel_,
                          playbackSlider_,
                          durationLabel_,
                          mirrorButton_,
                          smoothButton_,
                          overlayButton_};

    for (QWidget* widget : widgets)
    {
        auto sp = widget->sizePolicy();
        sp.setRetainSizeWhenHidden(true);
        widget->setSizePolicy(sp);
    }

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

    auto* leftLayout = new QHBoxLayout;
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(6);
    leftLayout->addLayout(mediaLayout);

    auto* centerLayout = new QHBoxLayout;
    centerLayout->setSpacing(6);

    centerLayout->addWidget(positionLabel_);
    centerLayout->addWidget(playbackSlider_, 1);
    centerLayout->addWidget(durationLabel_);

    auto* rightLayout = new QHBoxLayout;
    rightLayout->setSpacing(6);

    rightLayout->addWidget(mirrorButton_);
    rightLayout->addWidget(smoothButton_);
    rightLayout->addWidget(overlayButton_);

    auto* layout = new QHBoxLayout;
    layout->setContentsMargins(18, 10, 18, 10);
    layout->setSpacing(14);

    layout->addLayout(leftLayout);
    layout->addLayout(centerLayout, 1);
    layout->addLayout(rightLayout);

    background->setLayout(layout);

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(background);
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
