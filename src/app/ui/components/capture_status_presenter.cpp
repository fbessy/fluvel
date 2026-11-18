// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "capture_status_presenter.hpp"

#include "capture_controller.hpp"

#ifdef FLUVEL_USE_FFMPEG
#include "capture_stats_utils.hpp"
#endif

#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QStatusBar>

#include <cassert>

namespace fluvel
{

namespace
{
constexpr int kStatusMessageTimeoutMs = 5000;
}

CaptureStatusPresenter::CaptureStatusPresenter(QMainWindow* window, CaptureController* controller)
    : QObject(window), window_(window), controller_(controller)
{
    assert(window_ && controller_);
    assert(window_->statusBar());

#ifdef FLUVEL_USE_FFMPEG
    recordingStatsLabel_ = new QLabel(window_);
    recordingStatsLabel_->hide();
    window_->statusBar()->addPermanentWidget(recordingStatsLabel_);

    connect(controller_, &CaptureController::recordingStatsChanged, this,
            &CaptureStatusPresenter::onRecordingStatsChanged);

    connect(controller_, &CaptureController::recordingWarning, this,
            &CaptureStatusPresenter::onRecordingWarning);

    connect(controller_, &CaptureController::recordingError, this,
            &CaptureStatusPresenter::onRecordingError);

    connect(controller_, &CaptureController::recordingStarted, this,
            &CaptureStatusPresenter::onRecordingStarted);

    connect(controller_, &CaptureController::recordingFinalized, this,
            &CaptureStatusPresenter::onRecordingFinalized);
#endif

    connect(controller_, &CaptureController::snapshotSaved, this,
            &CaptureStatusPresenter::onSnapshotSaved);

    connect(controller_, &CaptureController::snapshotError, this,
            &CaptureStatusPresenter::onSnapshotError);
}

void CaptureStatusPresenter::onSnapshotSaved(const QString& filename)
{
    window_->statusBar()->showMessage(tr("Snapshot saved: %1").arg(filename),
                                      kStatusMessageTimeoutMs);
}

void CaptureStatusPresenter::onSnapshotError(const QString& message)
{
    window_->statusBar()->showMessage(message, kStatusMessageTimeoutMs);
}

#ifdef FLUVEL_USE_FFMPEG

void CaptureStatusPresenter::clearStatsLabel()
{
    recordingStatsLabel_->clear();
    recordingStatsLabel_->hide();
}

void CaptureStatusPresenter::onRecordingStarted(const QString& outputPath)
{
    window_->statusBar()->showMessage(tr("Recording to: %1").arg(outputPath),
                                      kStatusMessageTimeoutMs);
}

void CaptureStatusPresenter::onRecordingFinalized(const QString& outputPath)
{
    clearStatsLabel();

    window_->statusBar()->showMessage(tr("Recording saved: %1").arg(outputPath),
                                      kStatusMessageTimeoutMs);
}

void CaptureStatusPresenter::onRecordingWarning(const QString& message)
{
    QMessageBox::warning(window_, tr("Recording warning"), message);
}

void CaptureStatusPresenter::onRecordingError(const QString& message)
{
    QMessageBox::critical(window_, tr("Recording error"), message);
}

void CaptureStatusPresenter::onRecordingStatsChanged(const RecorderStats& stats)
{
    const auto status = capture_utils::formatRecordingStatus(controller_->recordingState(), stats);

    if (status.text.isEmpty())
    {
        clearStatsLabel();
        return;
    }

    recordingStatsLabel_->setText(status.text);
    recordingStatsLabel_->show();
}

#endif

} // namespace fluvel
