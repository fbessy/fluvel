// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "application_settings_types.hpp"
#include "image_settings_controller.hpp"

#include <QDialog>
#include <QImage>
#include <QPoint>

class QComboBox;
class QDialogButtonBox;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QRadioButton;
class QSlider;
class QSpinBox;
class QTabWidget;
class QWidget;

class QCloseEvent;
class QShowEvent;

namespace fluvel_app
{
class ImageViewerWidget;
class InitializationBehavior;
class KernelSizeSpinBox;
class AlgoSettingsWidget;

/**
 * @brief Tab indices for the settings dialog.
 */
enum class TabIndex
{
    Downscale,
    Preprocessing,
    Initialization,
    Algorithm
};

/**
 * @brief Dialog for configuring image processing and segmentation settings.
 *
 * This dialog allows the user to configure all aspects of an image session,
 * including:
 * - downscaling
 * - preprocessing filters and noise
 * - initialization shapes
 * - segmentation algorithm parameters
 *
 * The configuration is organized into multiple tabs and can be applied
 * or discarded via accept() / reject().
 *
 * It also provides interactive shape initialization through an
 * ImageViewerWidget and InitializationBehavior.
 *
 * @note The dialog maintains both committed and edited configurations.
 */
class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the settings dialog.
     *      * @param config Initial image session configuration.
     * @param parent Optional parent widget.
     */
    SettingsWindow(const ImageSessionSettings& config, QWidget* parent = nullptr);

    /**
     * @brief Updates the dialog with a new input image.
     *      * @param inputImage Input image used for preview and initialization.
     */
    void handleInputImageReady(const QImage& inputImage);

signals:
    /**
     * @brief Emitted when the configuration is accepted.
     */
    void imageSessionSettingsAccepted(const fluvel_app::ImageSessionSettings& config);

    /**
     * @brief Emitted when initialization mode is toggled.
     */
    void initializationModeChanged(bool enabled);

    /**
     * @brief Requests overlay update in the viewer.
     */
    void updateOverlay(const fluvel_app::UiShapeInfo& uiShape);

protected:
    /**
     * @brief Applies the edited configuration.
     */
    void accept() override;

    /**
     * @brief Restores UI values from the committed configuration.
     */
    void reject() override;

    /**
     * @brief Handles dialog show events.
     */
    void showEvent(QShowEvent* event) override;

    /**
     * @brief Handles dialog close events.
     */
    void closeEvent(QCloseEvent* event) override;

private:
    // slots
    void onTabChanged(int index);
    void onPreviewShapeAt(QPoint position);
    void onResizeShape(int delta);
    void onToggleShape();

    // slots
    void onAddShape();
    void onAddShapeAt(QPoint position);
    void onSubtractShape();
    void onSubtractShapeAt(QPoint position);
    void onClearPhi();
    void onUiShapeChanged();

    // other methods

    UiShapeInfo getUiShape() const;
    UiShapeInfo getUiShapeAt(QPoint position) const;
    QPoint uiPositionFromView(const QPoint& viewPosition) const;

    void setupUiDownscaleTab();
    void setupUiPreprocessingTab();
    void setupUiInitTab();
    void setupUiAlgoTab();

    void setupConnections();

    void updateUIFromConfig();
    void notifyConfigEdited();
    void restoreDefaults();

    ///////////////////////////////////////
    /////////////  UI Data  ///////////////
    ///////////////////////////////////////

    QTabWidget* tabs_ = nullptr;

    /////////////////////////////////////////

    QGroupBox* downscalePage_ = nullptr;
    QComboBox* downscaleFactorCb_ = nullptr;

    /////////////////////////////////////////

    QGroupBox* processPage_ = nullptr;
    QTabWidget* processInnerTabs_ = nullptr;

    QGroupBox* gaussianNoiseGroupbox_ = nullptr;
    QDoubleSpinBox* gaussianNoiseStdSpin_ = nullptr;

    QGroupBox* impulsiveNoiseGroupbox_ = nullptr;
    QDoubleSpinBox* impulsiveNoisePercentSpin_ = nullptr;

    QGroupBox* speckleNoiseGroupbox_ = nullptr;
    QDoubleSpinBox* speckleNoiseStdSpin_ = nullptr;

    QGroupBox* medianGroupbox_ = nullptr;
    KernelSizeSpinBox* medianKernelSizeSpin_ = nullptr;

    QGroupBox* meanGroupbox_ = nullptr;
    KernelSizeSpinBox* meanKernelSizeSpin_ = nullptr;

    QGroupBox* anisoGroupbox_ = nullptr;
    QRadioButton* anisoExpConductionRadio_ = nullptr;
    QRadioButton* anisoReciprocalConductionRadio_ = nullptr;
    QSpinBox* iterationFilterSpin_ = nullptr;
    QDoubleSpinBox* lambdaSpin_ = nullptr;
    QDoubleSpinBox* kappaSpin_ = nullptr;

    QGroupBox* openGroupbox_ = nullptr;
    KernelSizeSpinBox* openKernelSizeSpin_ = nullptr;

    QGroupBox* closeGroupbox_ = nullptr;
    KernelSizeSpinBox* closeKernelSizeSpin_ = nullptr;

    QGroupBox* tophatGroupbox_ = nullptr;
    QRadioButton* whitetophatRadio_ = nullptr;
    QRadioButton* blacktophatRadio_ = nullptr;
    KernelSizeSpinBox* tophatKernelSizeSpin_ = nullptr;

    QLabel* timeFilt_ = nullptr;

    /////////////////////////////////////////

    QWidget* initPage_ = nullptr;

    QRadioButton* rectangleRadio_ = nullptr;
    QRadioButton* ellipseRadio_ = nullptr;
    QSpinBox* widthShapeSpin_ = nullptr;
    QSlider* widthSlider_ = nullptr;
    QSpinBox* heightShapeSpin_ = nullptr;
    QSlider* heightSlider_ = nullptr;
    QSpinBox* abscissaSpin_ = nullptr;
    QSlider* abscissaSlider_ = nullptr;
    QSpinBox* ordinateSpin_ = nullptr;
    QSlider* ordinateSlider_ = nullptr;

    QPushButton* addButton_ = nullptr;
    QPushButton* subtractButton_ = nullptr;
    QPushButton* clearButton_ = nullptr;

    /////////////////////////////////////////

    QWidget* algoPage_ = nullptr;
    AlgoSettingsWidget* algoWidget_ = nullptr;

    /////////////////////////////////////////

    QDialogButtonBox* dialogButtons_ = nullptr;

    ////////////////////////////////////////
    /////////    View - Controller    /////
    ///////////////////////////////////////

    ImageViewerWidget* imageViewer_ = nullptr;
    InitializationBehavior* initializationBehavior_ = nullptr;
    ImageSettingsController* imageSettingsController_ = nullptr;
    int wheelAccumulator_ = 0;

    ////////////////////////////////////////
    /////////       Model              /////
    ///////////////////////////////////////

    ImageSessionSettings committedConfig_;

    DownscaleParams editedDownscaleParams_;
    fluvel_ip::ProcessingParams editedProcessingParams_;
};

} // namespace fluvel_app
