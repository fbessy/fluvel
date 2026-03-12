// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "common_settings.hpp"
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
class ImageView;
class InitializationBehavior;
class KernelSizeSpinBox;
class AlgoSettingsWidget;

enum class TabIndex
{
    Downscale,
    Preprocessing,
    Initialization,
    Algorithm
};

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    SettingsWindow(QWidget* parent, const ImageSessionSettings& config);

    void handleInputImageReady(const QImage& inputImage);

signals:
    void imageSessionSettingsAccepted(const fluvel_app::ImageSessionSettings& config);
    void initializationModeChanged(bool enabled);
    void updateOverlay(fluvel_app::UiShapeInfo uiShape);

protected:
    //! Save the configuration chosen into the ApplicationSettings.
    void accept() override;

    //! Restore the ui states in function of the ApplicationSettings.
    void reject() override;

    void showEvent(QShowEvent* event) override;
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

    QGroupBox* preprocessPage_ = nullptr;
    QTabWidget* preprocessTabs_ = nullptr;

    QGroupBox* gaussianNoiseGroupbox_ = nullptr;
    QDoubleSpinBox* gaussianNoiseStdSpin_ = nullptr;

    QGroupBox* saltNoiseGroupbox_ = nullptr;
    QDoubleSpinBox* saltNoisePercentSpin_ = nullptr;

    QGroupBox* speckleNoiseGroupbox_ = nullptr;
    QDoubleSpinBox* speckleNoiseStdSpin_ = nullptr;

    QGroupBox* medianGroupbox_ = nullptr;
    KernelSizeSpinBox* medianKernelSizeSpin_ = nullptr;
    QRadioButton* medianDirectRadio_ = nullptr;
    QRadioButton* medianPerreaultRadio_ = nullptr;

    QGroupBox* meanGroupbox_ = nullptr;
    KernelSizeSpinBox* meanKernelSizeSpin_ = nullptr;

    QGroupBox* gaussianGroupbox_ = nullptr;
    KernelSizeSpinBox* gaussianKernelSizeSpin_ = nullptr;
    QDoubleSpinBox* gaussianSigmaSpin_ = nullptr;

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

    QGroupBox* algoGroupbox_ = nullptr;
    QRadioButton* naiveRadio_ = nullptr;
    QRadioButton* perreaultRadio_ = nullptr;

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

    QDialogButtonBox* dialButtons_ = nullptr;

    ////////////////////////////////////////
    /////////    View - Controller    /////
    ///////////////////////////////////////

    ImageView* settingsView_ = nullptr;
    InitializationBehavior* initializationBehavior_ = nullptr;
    ImageSettingsController* imageSettingsController_ = nullptr;
    int wheelAccumulator_ = 0;

    ////////////////////////////////////////
    /////////       Model              /////
    ///////////////////////////////////////

    ImageSessionSettings committedConfig_;

    DownscaleConfig editedDownscaleConfig_;
    ProcessingConfig editedProcessingConfig_;
};

} // namespace fluvel_app
