// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "capture_controls_widget.hpp"
#include "animated_push_button.hpp"
#include "capture_controller.hpp"
#include "ffmpeg_codec_utils.hpp"
#include "icon_loader.hpp"
#include "morph_button.hpp"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QSettings>
#include <QSignalBlocker>

namespace
{

// Indices into the shapes given to MorphButton::setShapes().
enum RecordingShape
{
    ShapeStopped = 0,
    ShapeRecording = 1,
    ShapeDraining = 2
};

} // namespace

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
        recordingButton_ = new MorphButton(this);
        recordingButton_->setCheckable(true);
        recordingButton_->setEnabled(false);

        const QColor redRecording = QColor::fromRgb(0xFF453A);

        // One shape per state: size ratio, corner radius ratio, color.
        // An empty color follows the button text color.
        recordingButton_->setShapes({
            {0.75, 0.50, {}},           // Stopped: disk
            {0.75, 0.15, redRecording}, // Recording: rounded square
            {0.45, 0.15, redRecording}  // Draining: small rounded square
        });

        layout->insertWidget(0, recordingButton_);

        // Shown only by windows that support it (see setAutoRecordAvailable()).
        autoRecordCheck_ = new QCheckBox(tr("Auto"), this);
        autoRecordCheck_->setToolTip(tr("Start recording automatically when a new stream starts."));
        autoRecordCheck_->setVisible(false);

        layout->insertWidget(0, autoRecordCheck_);
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

    if (autoRecordCheck_ != nullptr)
    {
        connect(autoRecordCheck_, &QCheckBox::toggled, this,
                &CaptureControlsWidget::onAutoRecordToggled);
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

#ifdef FLUVEL_USE_FFMPEG
    applyAutoRecord();
#endif

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

void CaptureControlsWidget::setAutoRecordAvailable(bool available, const QString& settingsKey,
                                                   const QString& toolTip)
{
    if (autoRecordCheck_ == nullptr)
        return;

    autoRecordSettingsKey_ = settingsKey;

    if (!toolTip.isEmpty())
        autoRecordCheck_->setToolTip(toolTip);

    autoRecordAvailable_ = available;
    autoRecordCheck_->setVisible(available);

    {
        const QSignalBlocker blocker(autoRecordCheck_);

        autoRecordCheck_->setChecked(available &&
                                     QSettings().value(autoRecordSettingsKey_, false).toBool());
    }

    applyAutoRecord();
}

void CaptureControlsWidget::onAutoRecordToggled(bool checked)
{
    if (!autoRecordSettingsKey_.isEmpty())
        QSettings().setValue(autoRecordSettingsKey_, checked);

    applyAutoRecord();
}

void CaptureControlsWidget::applyAutoRecord()
{
    if (controller_ != nullptr && autoRecordCheck_ != nullptr)
        controller_->setAutoRecord(autoRecordAvailable_ && autoRecordCheck_->isChecked());
}

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
            recordingButton_->setShapeIndex(ShapeStopped);
            recordingButton_->setToolTip(tr("Start video recording."));
            break;

        case RecorderState::Recording:
            recordingButton_->setChecked(true);
            recordingButton_->setShapeIndex(ShapeRecording);
            recordingButton_->setToolTip(tr("Stop video recording."));
            break;

        case RecorderState::Draining:
            recordingButton_->setChecked(true);
            recordingButton_->setShapeIndex(ShapeDraining);
            recordingButton_->setToolTip(tr("Finalizing video recording..."));
            break;
    }

    updateRecordingButton();
}

#endif

} // namespace fluvel