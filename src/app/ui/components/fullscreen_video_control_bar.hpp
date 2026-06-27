// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "styled_tool_button.hpp"

#include <QWidget>

class QLabel;
class QComboBox;

namespace fluvel
{

class TimelineSlider;
class VolumeSlider;
class ClickableLabel;
class VolumeControlWidget;

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
    StyledToolButton* startStopButton() const;

    /**
     * @brief Returns the play/pause button.
     *
     * @return Play/pause button.
     */
    StyledToolButton* playPauseButton() const;

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
     * @brief Returns the volume control widget.
     *
     * @return Volume control widget.
     */
    VolumeControlWidget* volumeControl() const;

    /**
     * @brief Returns the mirror mode toggle button.
     *      * This checkable button enables or disables the selfie mirror effect.
     *      * @return Pointer to the mirror mode button.
     */
    StyledToolButton* mirrorButton() const;

    /**
     * @brief Returns the smooth display toggle button.
     *      * This checkable button enables or disables smooth image rendering.
     *      * @return Pointer to the smooth display button.
     */
    StyledToolButton* smoothButton() const;

    /**
     * @brief Returns the algorithm overlay toggle button.
     *      * This checkable button controls the visibility of the segmentation overlay.
     *      * @return Pointer to the overlay button.
     */
    StyledToolButton* overlayButton() const;

    /**
     * @brief Returns the camera selection combo box.
     *      * The combo box lists the available camera devices and is intended for
     * fullscreen camera mode.
     *      * @return Pointer to the camera selector.
     */
    QComboBox* cameraSelector() const;

private:
    StyledToolButton* startStopButton_{nullptr};
    StyledToolButton* playPauseButton_{nullptr};
    VolumeControlWidget* volumeControl_{nullptr};

    StyledToolButton* mirrorButton_{nullptr};
    StyledToolButton* smoothButton_{nullptr};
    StyledToolButton* overlayButton_{nullptr};

    TimelineSlider* playbackSlider_{nullptr};

    QLabel* positionLabel_{nullptr};
    ClickableLabel* durationLabel_{nullptr};

    QComboBox* cameraSelector_{nullptr};
};

} // namespace fluvel