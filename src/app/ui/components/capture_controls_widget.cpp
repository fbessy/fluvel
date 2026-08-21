// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "capture_controls_widget.hpp"
#include "animated_push_button.hpp"
#include "capture_controller.hpp"
#include "ffmpeg_codec_utils.hpp"
#include "icon_loader.hpp"

#include <QHBoxLayout>

namespace fluvel
{

CaptureControlsWidget::CaptureControlsWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);

    layout->setContentsMargins(0, 0, 0, 0);

    snapshotButton_ = new AnimatedPushButton(this);
    snapshotButton_->setEnabled(false);

    snapshotButton_->setIcon(
        il::loadIcon(QIcon::ThemeIcon::CameraPhoto, ":icons/actions/camera-photo-symbolic.svg"));

    snapshotButton_->setToolTip(tr("Take snapshot"));

    layout->addWidget(snapshotButton_);

#ifdef FLUVEL_USE_FFMPEG

    if (hasRecordingSupport())
    {
        recordingButton_ = new AnimatedPushButton(this);
        recordingButton_->setCheckable(true);
        recordingButton_->setEnabled(false);

        const QColor redRecording = QColor::fromRgb(0xFF453A);

        stoppedIcon_ = il::createDisk(palette().color(QPalette::WindowText));
        recordingIcon_ = il::createSquare(redRecording);
        drainingIcon_ = il::createSmallSquare(redRecording);

        recordingButton_->setIcon(stoppedIcon_);

        layout->insertWidget(0, recordingButton_);
    }

#endif

    layout->addStretch();

    connect(snapshotButton_, &QPushButton::clicked, this,
            [this]
            {
                if (controller_ != nullptr)
                    controller_->requestSnapshot();
            });

#ifdef FLUVEL_USE_FFMPEG

    if (recordingButton_ != nullptr)
    {
        connect(recordingButton_, &QPushButton::clicked, this,
                &CaptureControlsWidget::onToggleRecording);
    }

#endif
}

void CaptureControlsWidget::setCaptureController(CaptureController* controller)
{
    if (controller_ == controller)
        return;

    if (controller_ != nullptr)
        disconnect(controller_, nullptr, this, nullptr);

    controller_ = controller;

    if (controller_ == nullptr)
        return;

    connect(controller_, &CaptureController::streamingChanged, this,
            &CaptureControlsWidget::onStreamingChanged);

#ifdef FLUVEL_USE_FFMPEG

    connect(controller_, &CaptureController::recordingStateChanged, this,
            &CaptureControlsWidget::onRecordingStateChanged);

#endif

    onStreamingChanged(controller_->isStreaming());

#ifdef FLUVEL_USE_FFMPEG
    onRecordingStateChanged(controller_->recordingState());
#endif
}

#ifdef FLUVEL_USE_FFMPEG

void CaptureControlsWidget::onToggleRecording()
{
    if (controller_ == nullptr)
        return;

    if (controller_->isRecording())
        controller_->stopRecording();
    else
        controller_->startRecording();
}

#endif

void CaptureControlsWidget::onStreamingChanged(bool)
{
#ifdef FLUVEL_USE_FFMPEG
    updateRecordingButton();
#endif

    updateSnapshotButton();
}

void CaptureControlsWidget::updateSnapshotButton()
{
    snapshotButton_->setEnabled(controller_ != nullptr && controller_->isStreaming());
}

#ifdef FLUVEL_USE_FFMPEG

bool CaptureControlsWidget::hasRecordingSupport() const
{
    return !FFmpegCodecUtils::availableCodecs().isEmpty();
}

void CaptureControlsWidget::updateRecordingButton()
{
    if (recordingButton_ == nullptr)
        return;

    recordingButton_->setEnabled(controller_ != nullptr && controller_->isStreaming());
}

#endif

#ifdef FLUVEL_USE_FFMPEG

void CaptureControlsWidget::onRecordingStateChanged(RecorderState state)
{
    if (recordingButton_ == nullptr)
        return;

    switch (state)
    {
        case RecorderState::Stopped:
            recordingButton_->setChecked(false);
            recordingButton_->setIcon(stoppedIcon_);
            recordingButton_->setToolTip(tr("Start video recording."));
            break;

        case RecorderState::Recording:
            recordingButton_->setChecked(true);
            recordingButton_->setIcon(recordingIcon_);
            recordingButton_->setToolTip(tr("Stop video recording."));
            break;

        case RecorderState::Draining:
            recordingButton_->setChecked(true);
            recordingButton_->setIcon(drainingIcon_);
            recordingButton_->setToolTip(tr("Finalizing video recording..."));
            break;
    }

    updateRecordingButton();
}

#endif

} // namespace fluvel