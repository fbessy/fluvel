// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QObject>

class QWidget;
class QShortcut;

namespace fluvel
{

/**
 * @brief Handles keyboard shortcuts for video playback.
 *
 * VideoShortcutManager centralizes the keyboard shortcuts associated
 * with media playback and emits high-level requests corresponding to
 * user actions.
 *
 * The actual playback logic is implemented by the caller.
 */
class VideoShortcutManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a video shortcut manager.
     *
     * @param parent Parent widget receiving the shortcuts.
     */
    explicit VideoShortcutManager(QWidget* parent);

    /**
     * @brief Enables or disables all managed shortcuts.
     *
     * @param enabled @c true to enable the shortcuts, @c false to disable them.
     */
    void setEnabled(bool enabled);

signals:
    /**
     * @brief Emitted when the user requests to toggle playback.
     */
    void playPauseRequested();

    /**
     * @brief Emitted when the user requests to seek relatively.
     *
     * @param offsetMs Relative seek offset, in milliseconds.
     *                 Positive values seek forward, negative values seek backward.
     */
    void seekRequested(qint64 offsetMs);

    /**
     * @brief Emitted when the user requests to change the playback volume.
     *
     * @param deltaPercent Relative volume change, in percent.
     *                     Positive values increase the volume, negative values decrease it.
     */
    void volumeRequested(int deltaPercent);

    /**
     * @brief Emitted when the user requests to toggle the muted state.
     */
    void toggleMuteRequested();

    /**
     * @brief Emitted when the user requests to toggle fullscreen mode.
     */
    void toggleFullscreenRequested();

    /**
     * @brief Emitted when the user requests to leave fullscreen mode.
     */
    void escapeRequested();

private:
    /**
     * @brief Creates and configures the keyboard shortcuts.
     *
     * @param parent Parent widget receiving the shortcuts.
     */
    void createShortcuts(QWidget* parent);

    QShortcut* playPauseShortcut_{nullptr};
    QShortcut* seekForwardShortcut_{nullptr};
    QShortcut* seekBackwardShortcut_{nullptr};
    QShortcut* volumeUpShortcut_{nullptr};
    QShortcut* volumeDownShortcut_{nullptr};
    QShortcut* muteShortcut_{nullptr};
    QShortcut* fullscreenShortcut_{nullptr};
    QShortcut* escapeShortcut_{nullptr};
};

} // namespace fluvel