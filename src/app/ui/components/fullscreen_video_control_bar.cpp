// SPDX-License-Identifier: CeCILL-2.1

#include "fullscreen_video_control_bar.hpp"

#include "clickable_label.hpp"
#include "icon_loader.hpp"
#include "timeline_slider.hpp"
#include "ui_theme.hpp"
#include "volume_control_widget.hpp"

#include <QColor>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QPropertyAnimation>
#include <QWidget>

namespace fluvel
{

static QString rgba(const QColor& c)
{
    return QString("rgba(%1,%2,%3,%4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
}

const QColor kAccentColor(107, 111, 207, 220);
const QColor kAccentHover = kAccentColor.lighter(110);

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
    opacity: 0.8;
}
)");

    setStyleSheet(R"(
QLabel {
    color: white;
}
QComboBox {
    min-width: 220px;
}

QLabel:disabled
{
    color: rgba(255,255,255,70);
}

QComboBox
{
    color: white;

    background-color: rgba(255,255,255,20);

    border: 1px solid rgba(255,255,255,35);
    border-radius: 15px;

    padding: 4px 28px 4px 10px;

    min-height: 30px;
}

QComboBox:focus
{
    border: 1px solid #8B5CF6;
}

QComboBox:hover
{
    background-color: rgba(255,255,255,30);
}

QComboBox::drop-down
{
    border: none;
    width: 24px;
}

QComboBox::down-arrow
{
    image: none;
}

QComboBox QAbstractItemView
{
    background-color: rgb(32,36,42);
    color: white;

    border: none;

    outline: none;

    padding: 4px;
}

QComboBox QAbstractItemView
{
    background: rgb(32,36,42);
    border: 1px solid rgb(60,60,60);
    border-radius: 12px;
    selection-background-color: rgba(255,255,255,25);
}

QComboBox QAbstractItemView::item:selected
{
    background-color: rgba(255,255,255,25);
    color: white;
}

QComboBox QAbstractItemView::item:hover
{
    background-color: rgba(255,255,255,15);
}

QListView
{
    background: rgb(32,36,42);
    border: none;
}

QScrollBar:vertical
{
    width: 0px;
    border: none;
    background: transparent;
}

QScrollBar:horizontal
{
    height: 0px;
    border: none;
    background: transparent;
}

QListView::viewport
{
    background: rgb(32,36,42);
}

)");

    cameraSelector_ = new QComboBox;
    auto* view = new QListView(cameraSelector_);

    view->setFrameShape(QFrame::NoFrame);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    view->viewport()->setAutoFillBackground(false);
    view->setAutoFillBackground(false);
    view->viewport()->setStyleSheet("background: rgb(32,36,42);");
    view->setContentsMargins(0, 0, 0, 0);
    view->viewport()->setContentsMargins(0, 0, 0, 0);
    view->setAttribute(Qt::WA_StyledBackground);

    cameraSelector_->setView(view);

    startStopButton_ = new StyledToolButton(this, ui::Appearance::Modern);
    playPauseButton_ = new StyledToolButton(this, ui::Appearance::Modern);
    volumeControl_ = new VolumeControlWidget(this, ui::Appearance::Modern);

    mirrorButton_ = new StyledToolButton(this, ui::Appearance::Modern);
    smoothButton_ = new StyledToolButton(this, ui::Appearance::Modern);
    overlayButton_ = new StyledToolButton(this, ui::Appearance::Modern);

    mirrorButton_->setCheckable(true);
    smoothButton_->setCheckable(true);
    overlayButton_->setCheckable(true);

    QIcon mirrorIcon = il::loadIcon(":/icons/view/mirror-symbolic.svg", il::IconMode::Light);

    QIcon smoothIcon =
        il::loadIcon(":/icons/actions/smooth_rendering-symbolic.svg", il::IconMode::Light);

    QIcon infoIcon = il::loadIcon(":/icons/actions/help-about-symbolic.svg", il::IconMode::Light);

    mirrorButton_->setIcon(mirrorIcon);
    smoothButton_->setIcon(smoothIcon);
    overlayButton_->setIcon(infoIcon);

    constexpr QSize kNormalIconSize(24, 24);
    constexpr QSize kCheckedIconSize(26, 26);

    for (StyledToolButton* b : {mirrorButton_, smoothButton_, overlayButton_})
    {
        connect(b, &QToolButton::toggled, this,
                [b, kNormalIconSize, kCheckedIconSize](bool checked)
                {
                    b->setIconSize(checked ? kCheckedIconSize : kNormalIconSize);
                });
    }

    positionLabel_ = new QLabel("00:00");
    durationLabel_ = new ClickableLabel("-00:00");

    QFont f = positionLabel_->font();
    f.setBold(true);
    f.setPointSize(14);
    positionLabel_->setFont(f);
    durationLabel_->setFont(f);

    playbackSlider_ = new TimelineSlider(this, ui::Appearance::Modern);
    playbackSlider_->setMinimumWidth(260);
    playbackSlider_->setMaximumWidth(520);
    playbackSlider_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    const auto bSize = ui::kButtonSize;
    const auto iSize = ui::kButtonIconSize;

    constexpr QSize kIconSize(iSize, iSize);

    for (auto* b :
         {startStopButton_, playPauseButton_, mirrorButton_, smoothButton_, overlayButton_})
    {
        b->setIconSize(kIconSize);
        b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        b->setFixedSize(bSize, bSize);
        b->setCursor(Qt::PointingHandCursor);
        // b->setAutoRaise(true);
    }

    QWidget* widgets[] = {startStopButton_, playPauseButton_, volumeControl_,
                          positionLabel_,   playbackSlider_,  durationLabel_,
                          mirrorButton_,    smoothButton_,    overlayButton_};

    for (QWidget* w : widgets)
    {
        auto sp = w->sizePolicy();
        sp.setRetainSizeWhenHidden(true);
        w->setSizePolicy(sp);
    }

    auto* leftLayout = new QHBoxLayout;
    leftLayout->setSpacing(6);

    leftLayout->addWidget(cameraSelector_);
    leftLayout->addWidget(startStopButton_);
    leftLayout->addWidget(playPauseButton_);
    leftLayout->addWidget(volumeControl_);

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
    layout->addLayout(centerLayout);
    layout->addLayout(rightLayout);

    background->setLayout(layout);

    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(background);
}

StyledToolButton* FullscreenVideoControlBar::startStopButton() const
{
    return startStopButton_;
}
StyledToolButton* FullscreenVideoControlBar::playPauseButton() const
{
    return playPauseButton_;
}
VolumeControlWidget* FullscreenVideoControlBar::volumeControl() const
{
    return volumeControl_;
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
QComboBox* FullscreenVideoControlBar::cameraSelector() const
{
    return cameraSelector_;
}
TimelineSlider* FullscreenVideoControlBar::playbackSlider() const
{
    return playbackSlider_;
}
QLabel* FullscreenVideoControlBar::positionLabel() const
{
    return positionLabel_;
}
ClickableLabel* FullscreenVideoControlBar::durationLabel() const
{
    return durationLabel_;
}

} // namespace fluvel
