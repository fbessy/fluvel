// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QWidget>

class QLabel;
class QPushButton;

namespace fluvel
{

class TimelineSlider;
class ClickableLabel;

/**
 * @brief Fullscreen media control bar.
 *
 * Provides access to the main media playback controls displayed
 * when viewing videos in fullscreen mode.
 *
 * The bar contains playback controls, a timeline slider, time
 * indicators and volume controls.
 */
class FullscreenVideoControlBar : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a fullscreen video control bar.
     *
     * @param parent Parent widget.
     */
    explicit FullscreenVideoControlBar(QWidget* parent = nullptr);

    /**
     * @brief Returns the stream start/stop button.
     *
     * @return Start/stop button.
     */
    QPushButton* startStopButton() const;

    /**
     * @brief Returns the play/pause button.
     *
     * @return Play/pause button.
     */
    QPushButton* playPauseButton() const;

    /**
     * @brief Returns the playback timeline slider.
     *
     * @return Playback slider.
     */
    TimelineSlider* playbackSlider() const;

    /**
     * @brief Returns the current playback position label.
     *
     * @return Position label.
     */
    QLabel* positionLabel() const;

    /**
     * @brief Returns the media duration label.
     *
     * @return Duration label.
     */
    ClickableLabel* durationLabel() const;

    /**
     * @brief Returns the volume button.
     *
     * @return Volume button.
     */
    QPushButton* volumeButton() const;

private:
    QPushButton* startStopButton_{nullptr};
    QPushButton* playPauseButton_{nullptr};

    TimelineSlider* playbackSlider_{nullptr};

    QLabel* positionLabel_{nullptr};
    ClickableLabel* durationLabel_{nullptr};

    QPushButton* volumeButton_{nullptr};
};

} // namespace fluvel