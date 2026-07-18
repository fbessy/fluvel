// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QDialog>

class QCheckBox;
class QCloseEvent;
class QComboBox;
class QLabel;
class QLineEdit;
class QWidget;
class QSpinBox;

namespace fluvel
{

/**
 * @brief Dialog for configuring persistent user preferences.
 *
 * The dialog allows the user to configure the application language,
 * snapshot output settings and video recording settings.
 */
class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the user preferences dialog.
     *
     * @param parent Optional parent widget.
     */
    explicit PreferencesDialog(QWidget* parent = nullptr);

protected:
    /**
     * @brief Applies the selected user preferences.
     */
    void accept() override;

    /**
     * @brief Handles dialog close events.
     *
     * @param event Close event.
     */
    void closeEvent(QCloseEvent* event) override;

private:
    /// Creates the language preferences section.
    QWidget* createLanguageSection();

    /// Creates the snapshot preferences section.
    QWidget* createSnapshotSection();

    /// Creates the video recording preferences section.
    QWidget* createVideoRecordingSection();

    /// Creates the recording buffer preferences section.
    QWidget* createRecordingBufferSection();

    /// Selects an output directory for snapshots.
    void selectSnapshotDirectory();

    /// Selects an output directory for video recordings.
    void selectVideoDirectory();

    /// Updates the snapshot filename preview.
    void updateSnapshotPreview();

    /// Updates the video recording filename preview.
    void updateVideoPreview();

    QComboBox* languageCombo_{nullptr};

    QLineEdit* snapshotDirectoryEdit_{nullptr};
    QLineEdit* snapshotBaseNameEdit_{nullptr};
    QComboBox* snapshotFormatCombo_{nullptr};
    QCheckBox* snapshotTimestampCheck_{nullptr};
    QLabel* snapshotPreviewLabel_{nullptr};

    QLineEdit* videoDirectoryEdit_{nullptr};
    QLineEdit* videoBaseNameEdit_{nullptr};
    QComboBox* videoCodecCombo_{nullptr};
    QCheckBox* videoTimestampCheck_{nullptr};
    QLabel* videoPreviewLabel_{nullptr};

    QSpinBox* recordingRamSpin_{nullptr};
    QSpinBox* recordingDiskSpin_{nullptr};
    QComboBox* recordingOverflowCombo_{nullptr};
    QSpinBox* recordingCircularDurationSpin_{nullptr};
};

} // namespace fluvel