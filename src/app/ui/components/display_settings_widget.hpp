// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#ifndef Q_MOC_RUN
#include "application_settings_types.hpp"
#endif

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QWidget>

namespace fluvel
{

class ColorSelectorWidget;

/**
 * @brief Widget for configuring display parameters of the image viewer.
 *
 * This widget provides UI controls to adjust how images and overlays
 * are rendered, including:
 * - pipeline source selection (raw / preprocessed)
 * - contour colors
 * - display options (flip, smoothing, overlay)
 *
 * It emits displayConfigChanged() whenever the configuration is updated.
 */
class DisplaySettingsWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the display settings widget.
     *
     * @param config Initial display configuration.
     * @param parent Optional parent widget.
     */
    DisplaySettingsWidget(const DisplayConfig& config, QWidget* parent = nullptr);

    /**
     * @brief Updates the availability of image display modes.
     *
     * When preprocessing is unavailable, the display mode is forced
     * back to the source image.
     *
     * @param hasPreprocessing True if a preprocessed image is available.
     */
    void updateDisplayModeAvailability(bool hasPreprocessing);

signals:
    /**
     * @brief Emitted when the display configuration changes.
     *
     * @param config Updated configuration.
     */
    void displayConfigChanged(const fluvel::DisplayConfig& config);

public slots:
    /**
     * @brief Shows or hides the settings panel.
     *
     * @param open True to show the panel, false to hide it.
     */
    void setPanelVisible(bool open);

private:
    /**
     * @brief Animates panel visibility changes.
     */
    void animate(bool open);

    bool isAnimating_{false};

    QGroupBox* displayModeGroupBox_ = nullptr;
    QRadioButton* sourceRadioButton_ = nullptr;
    QRadioButton* preprocessedRadioButton_ = nullptr;

    ColorSelectorWidget* outerContourColorSelector_ = nullptr;
    ColorSelectorWidget* innerContourColorSelector_ = nullptr;

    QCheckBox* mirrorModeCheckBox_ = nullptr;
    QCheckBox* smoothDisplayCheckBox_ = nullptr;

    QCheckBox* overlayCheckBox_ = nullptr;
    QCheckBox* miniMapCheckBox_ = nullptr;

    DisplayConfig config_{};
};

} // namespace fluvel
