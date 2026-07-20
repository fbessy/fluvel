// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "preferences_dialog.hpp"

#include "application_settings.hpp"
#include "file_utils.hpp"
#include "recording_buffer_settings.hpp"
#include "video_exporter.hpp"
#include "video_exporter_utils.hpp"

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QVariant>

namespace fluvel
{

namespace
{

QString filenamePreview(const QString& baseName, const QString& extension, bool appendTimestamp)
{
    QString name = baseName;

    if (appendTimestamp)
        name += "_2026-07-14_19-30-00";

    return name + "." + extension;
}

} // namespace

PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Preferences"));

    QSettings settings;

    const auto geometry = settings.value("ui_geometry/preferences_dialog").toByteArray();

    if (!geometry.isEmpty())
        restoreGeometry(geometry);

    auto* dialogButtons =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    dialogButtons->setCenterButtons(true);

    connect(dialogButtons, &QDialogButtonBox::accepted, this, &PreferencesDialog::accept);
    connect(dialogButtons, &QDialogButtonBox::rejected, this, &PreferencesDialog::reject);

    auto* layout = new QVBoxLayout(this);

    layout->addWidget(createLanguageSection());
    layout->addWidget(createSnapshotSection());
    layout->addWidget(createVideoRecordingSection());
    layout->addWidget(createRecordingBufferSection());

    layout->addWidget(dialogButtons);

    layout->setSizeConstraint(QLayout::SetMinimumSize);
}

QWidget* PreferencesDialog::createLanguageSection()
{
    auto* group = new QGroupBox(tr("Language"), this);
    auto* layout = new QVBoxLayout(group);

    languageCombo_ = new QComboBox(group);

    const QString locale = QLocale::system().name().section('_', 0, 0);

    languageCombo_->addItem(tr("System (%1)").arg(locale),
                            QVariant::fromValue(int(Language::System)));

    languageCombo_->addItem(tr("English"), QVariant::fromValue(int(Language::English)));
    languageCombo_->addItem(tr("French"), QVariant::fromValue(int(Language::French)));

    const Language language = ApplicationSettings::instance().appLanguage();

    const int index = languageCombo_->findData(QVariant::fromValue(int(language)));

    if (index >= 0)
        languageCombo_->setCurrentIndex(index);

    auto* restartLabel = new QLabel(
        tr("The language change will take effect after restarting the application."), group);

    restartLabel->setWordWrap(true);

    layout->addWidget(languageCombo_);
    layout->addWidget(restartLabel);

    return group;
}

QWidget* PreferencesDialog::createSnapshotSection()
{
    const auto& preferences = ApplicationSettings::instance().snapshotPreferences();

    auto* group = new QGroupBox(tr("Snapshots"), this);
    auto* layout = new QFormLayout(group);

    snapshotDirectoryEdit_ = new QLineEdit(preferences.directory, group);
    snapshotBaseNameEdit_ = new QLineEdit(preferences.baseName, group);
    snapshotFormatCombo_ = new QComboBox(group);
    snapshotTimestampCheck_ = new QCheckBox(tr("Append timestamp"), group);
    snapshotPreviewLabel_ = new QLabel(group);

    auto* browseButton = new QPushButton(tr("Browse..."), group);

    auto* directoryLayout = new QHBoxLayout;
    directoryLayout->addWidget(snapshotDirectoryEdit_, 1);
    directoryLayout->addWidget(browseButton);

    for (const QByteArray& format : file_utils::writableImageFormats())
    {
        snapshotFormatCombo_->addItem(QString::fromLatin1(format).toUpper(), format);
    }

    const int formatIndex = snapshotFormatCombo_->findData(preferences.preferredFormat);

    if (formatIndex >= 0)
        snapshotFormatCombo_->setCurrentIndex(formatIndex);

    snapshotTimestampCheck_->setChecked(preferences.appendTimestamp);

    layout->addRow(tr("Directory:"), directoryLayout);
    layout->addRow(tr("Base name:"), snapshotBaseNameEdit_);
    layout->addRow(tr("Format:"), snapshotFormatCombo_);
    layout->addRow(QString(), snapshotTimestampCheck_);
    layout->addRow(tr("Preview:"), snapshotPreviewLabel_);

    connect(browseButton, &QPushButton::clicked, this, &PreferencesDialog::selectSnapshotDirectory);

    connect(snapshotBaseNameEdit_, &QLineEdit::textChanged, this,
            &PreferencesDialog::updateSnapshotPreview);

    connect(snapshotFormatCombo_, &QComboBox::currentIndexChanged, this,
            &PreferencesDialog::updateSnapshotPreview);

    connect(snapshotTimestampCheck_, &QCheckBox::toggled, this,
            &PreferencesDialog::updateSnapshotPreview);

    updateSnapshotPreview();

    return group;
}

QWidget* PreferencesDialog::createVideoRecordingSection()
{
    const auto& preferences = ApplicationSettings::instance().videoRecordingPreferences();

    auto* group = new QGroupBox(tr("Video recording"), this);
    auto* layout = new QFormLayout(group);

    videoDirectoryEdit_ = new QLineEdit(preferences.directory, group);
    videoBaseNameEdit_ = new QLineEdit(preferences.baseName, group);
    videoCodecCombo_ = new QComboBox(group);
    videoTimestampCheck_ = new QCheckBox(tr("Append timestamp"), group);
    videoPreviewLabel_ = new QLabel(group);

    auto* browseButton = new QPushButton(tr("Browse..."), group);

    auto* directoryLayout = new QHBoxLayout;
    directoryLayout->addWidget(videoDirectoryEdit_, 1);
    directoryLayout->addWidget(browseButton);

    VideoExporter exporter;

    for (VideoCodec codec : exporter.availableCodecs())
    {
        videoCodecCombo_->addItem(exporter_utils::toString(codec), int(codec));
    }

    const int codecIndex = videoCodecCombo_->findData(int(preferences.preferredCodec));

    if (codecIndex >= 0)
        videoCodecCombo_->setCurrentIndex(codecIndex);

    videoTimestampCheck_->setChecked(preferences.appendTimestamp);

    layout->addRow(tr("Directory:"), directoryLayout);
    layout->addRow(tr("Base name:"), videoBaseNameEdit_);
    layout->addRow(tr("Codec:"), videoCodecCombo_);
    layout->addRow(QString(), videoTimestampCheck_);
    layout->addRow(tr("Preview:"), videoPreviewLabel_);

    connect(browseButton, &QPushButton::clicked, this, &PreferencesDialog::selectVideoDirectory);

    connect(videoBaseNameEdit_, &QLineEdit::textChanged, this,
            &PreferencesDialog::updateVideoPreview);

    connect(videoCodecCombo_, &QComboBox::currentIndexChanged, this,
            &PreferencesDialog::updateVideoPreview);

    connect(videoTimestampCheck_, &QCheckBox::toggled, this,
            &PreferencesDialog::updateVideoPreview);

    updateVideoPreview();

    return group;
}

QWidget* PreferencesDialog::createRecordingBufferSection()
{
    const auto& bufferSettings = ApplicationSettings::instance().recordingBufferSettings();

    auto* groupBox = new QGroupBox(tr("Recording buffer"));

    auto* layout = new QFormLayout(groupBox);

    recordingRamSpin_ = new QSpinBox(groupBox);
    recordingRamSpin_->setRange(128, 32768);
    recordingRamSpin_->setSuffix(tr(" MiB"));

    recordingDiskSpin_ = new QSpinBox(groupBox);
    recordingDiskSpin_->setRange(0, 1024);
    recordingDiskSpin_->setSuffix(tr(" GiB"));

    recordingOverflowCombo_ = new QComboBox(groupBox);
    recordingOverflowCombo_->addItem(tr("Stop recording"),
                                     int(BufferOverflowPolicy::StopRecording));

    // recordingOverflowCombo_->addItem(tr("Circular buffer"),
    // int(BufferOverflowPolicy::StopRecording));

    recordingRamSpin_->setValue(int(bufferSettings.maxRamUsage / (1024 * 1024)));

    recordingDiskSpin_->setValue(int(bufferSettings.maxDiskUsage / (1024 * 1024 * 1024)));

    recordingOverflowCombo_->setCurrentIndex(
        recordingOverflowCombo_->findData(int(bufferSettings.overflowPolicy)));

    layout->addRow(tr("Maximum RAM usage:"), recordingRamSpin_);
    layout->addRow(tr("Maximum temporary usage:"), recordingDiskSpin_);
    layout->addRow(tr("When buffer is full:"), recordingOverflowCombo_);

    connect(
        recordingOverflowCombo_, &QComboBox::currentIndexChanged, this,
        [this]
        {
            const auto policy =
                static_cast<BufferOverflowPolicy>(recordingOverflowCombo_->currentData().toInt());
        });

    return groupBox;
}

void PreferencesDialog::selectSnapshotDirectory()
{
    const QString directory = QFileDialog::getExistingDirectory(
        this, tr("Select snapshot directory"), snapshotDirectoryEdit_->text());

    if (!directory.isEmpty())
        snapshotDirectoryEdit_->setText(directory);
}

void PreferencesDialog::selectVideoDirectory()
{
    const QString directory = QFileDialog::getExistingDirectory(
        this, tr("Select video recording directory"), videoDirectoryEdit_->text());

    if (!directory.isEmpty())
        videoDirectoryEdit_->setText(directory);
}

void PreferencesDialog::updateSnapshotPreview()
{
    const QString extension =
        QString::fromLatin1(snapshotFormatCombo_->currentData().toByteArray());

    snapshotPreviewLabel_->setText(filenamePreview(snapshotBaseNameEdit_->text(), extension,
                                                   snapshotTimestampCheck_->isChecked()));
}

void PreferencesDialog::updateVideoPreview()
{
    if (videoCodecCombo_->currentIndex() < 0)
    {
        videoPreviewLabel_->clear();
        return;
    }

    const auto codec = static_cast<VideoCodec>(videoCodecCombo_->currentData().toInt());

    const VideoContainer container = exporter_utils::preferredContainer(codec);

    videoPreviewLabel_->setText(filenamePreview(videoBaseNameEdit_->text(),
                                                exporter_utils::expectedExtension(container),
                                                videoTimestampCheck_->isChecked()));
}

void PreferencesDialog::accept()
{
    auto& settings = ApplicationSettings::instance();

    settings.setAppLanguage(static_cast<Language>(languageCombo_->currentData().toInt()));

    SnapshotPreferences snapshotPreferences;
    snapshotPreferences.directory = snapshotDirectoryEdit_->text();
    snapshotPreferences.baseName = snapshotBaseNameEdit_->text();
    snapshotPreferences.preferredFormat = snapshotFormatCombo_->currentData().toByteArray();
    snapshotPreferences.appendTimestamp = snapshotTimestampCheck_->isChecked();

    settings.setSnapshotPreferences(snapshotPreferences);

    VideoRecordingPreferences videoPreferences;
    videoPreferences.directory = videoDirectoryEdit_->text();
    videoPreferences.baseName = videoBaseNameEdit_->text();

    if (videoCodecCombo_->currentIndex() >= 0)
    {
        videoPreferences.preferredCodec =
            static_cast<VideoCodec>(videoCodecCombo_->currentData().toInt());
    }

    videoPreferences.appendTimestamp = videoTimestampCheck_->isChecked();

    settings.setVideoRecordingPreferences(videoPreferences);

    auto bufferSettings = settings.recordingBufferSettings();

    bufferSettings.maxRamUsage = std::uint64_t(recordingRamSpin_->value()) * 1024 * 1024;

    bufferSettings.maxDiskUsage = std::uint64_t(recordingDiskSpin_->value()) * 1024 * 1024 * 1024;

    bufferSettings.overflowPolicy =
        static_cast<BufferOverflowPolicy>(recordingOverflowCombo_->currentData().toInt());

    settings.setRecordingBufferSettings(bufferSettings);

    settings.save();

    QDialog::accept();
}

void PreferencesDialog::closeEvent(QCloseEvent* event)
{
    QSettings settings;
    settings.setValue("ui_geometry/preferences_dialog", saveGeometry());

    QDialog::closeEvent(event);
}

} // namespace fluvel