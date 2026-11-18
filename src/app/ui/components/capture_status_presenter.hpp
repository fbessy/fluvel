// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "recording_types.hpp"

#include <QObject>
#include <QString>

class QLabel;
class QMainWindow;

namespace fluvel
{

class CaptureController;

/**
 * @brief Reports capture events (recording, snapshots) in a window.
 *
 * Shared by every window that owns a CaptureController. It listens to the
 * controller signals and reports them through the window status bar, a
 * permanent recording-statistics label, and message boxes for warnings
 * and errors.
 *
 * The presenter is parented to the window and must not outlive the
 * controller.
 */
class CaptureStatusPresenter : public QObject
{
    Q_OBJECT

public:
    /**
     * @param window     Window whose status bar is used and which parents
     *                   message boxes. Must not be null.
     * @param controller Source of the capture signals. Must not be null.
     */
    CaptureStatusPresenter(QMainWindow* window, CaptureController* controller);

private:
    void onSnapshotSaved(const QString& filename);
    void onSnapshotError(const QString& message);

#ifdef FLUVEL_USE_FFMPEG
    void onRecordingStatsChanged(const RecorderStats& stats);
    void onRecordingStarted(const QString& outputPath);
    void onRecordingFinalized(const QString& outputPath);
    void onRecordingWarning(const QString& message);
    void onRecordingError(const QString& message);

    void clearStatsLabel();

    QLabel* recordingStatsLabel_{nullptr};
#endif

    QMainWindow* window_{nullptr};
    CaptureController* controller_{nullptr};
};

} // namespace fluvel
