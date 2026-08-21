// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#ifdef FLUVEL_USE_FFMPEG
#include "recording_types.hpp"
#endif

#include <QIcon>
#include <QWidget>

class QPushButton;

namespace fluvel
{

class AnimatedPushButton;
class CaptureController;

/**
 * @brief Displays the capture controls.
 *
 * This widget provides the user interface for snapshot and video recording.
 * It reflects the current streaming and recording state and exposes the
 * corresponding capture actions to the user.
 *
 * All capture operations are delegated to a CaptureController. The widget is
 * responsible only for presenting the controls and reflecting the controller
 * state.
 */
class CaptureControlsWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the capture controls widget.
     *
     * @param parent Parent widget.
     */
    explicit CaptureControlsWidget(QWidget* parent = nullptr);

    /**
     * @brief Associates a capture controller with the widget.
     *
     * The widget automatically connects to the controller signals and updates
     * its controls to reflect the current capture state.
     *
     * Passing @c nullptr disconnects the current controller.
     *
     * @param controller Capture controller.
     */
    void setCaptureController(CaptureController* controller);

private slots:

#ifdef FLUVEL_USE_FFMPEG

    /**
     * @brief Starts or stops video recording.
     */
    void onToggleRecording();

    /**
     * @brief Updates the recording controls.
     *
     * @param state Current recorder state.
     */
    void onRecordingStateChanged(RecorderState state);

#endif

    /**
     * @brief Updates the controls according to the streaming state.
     *
     * @param streaming True if a stream is active.
     */
    void onStreamingChanged(bool streaming);

private:
    /**
     * @brief Updates the snapshot button state.
     */
    void updateSnapshotButton();

#ifdef FLUVEL_USE_FFMPEG

    /**
     * @brief Returns whether video recording is available.
     *
     * @return True if at least one usable video encoder is available.
     */
    bool hasRecordingSupport() const;

    /**
     * @brief Updates the recording button state.
     */
    void updateRecordingButton();

#endif

    CaptureController* controller_{nullptr};

    AnimatedPushButton* snapshotButton_{nullptr};

#ifdef FLUVEL_USE_FFMPEG

    AnimatedPushButton* recordingButton_{nullptr};

    QIcon stoppedIcon_;
    QIcon recordingIcon_;
    QIcon drainingIcon_;

#endif
};

} // namespace fluvel