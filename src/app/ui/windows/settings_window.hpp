// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "algo_settings_widget.hpp"
#include "filters.hpp"
#include "image_settings_controller.hpp"
#include "image_view.hpp"
#include "initialization_behavior.hpp"
#include "kernel_size_spinbox.hpp"

#include <QtWidgets>

namespace fluvel_app
{

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

public slots:
    void onTabChanged(int index);
    void onPreviewShapeAt(QPoint position);
    void onResizeShape(int delta);
    void onToggleShape();

    void onAddShape();
    void onAddShapeAt(QPoint position);
    void onSubtractShape();
    void onSubtractShapeAt(QPoint position);
    void onClearPhi();

    void onInputImageReady(const QImage& inputImage);

signals:
    void imageSessionSettingsAccepted(const fluvel_app::ImageSessionSettings& config);
    void initializationModeChanged(bool enabled);

protected:
    //! Save the configuration chosen into the ApplicationSettings.
    void accept() override;

    //! Restore the ui states in function of the ApplicationSettings.
    void reject() override;

    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

signals:
    void updateOverlay(fluvel_app::UiShapeInfo uiShape);

private:
    //////////////////////////////////////////
    //   pour la fenêtre de configuration   //
    /////////////////////////////////////////

    void onUiShapeChanged();
    UiShapeInfo getUiShape() const;
    UiShapeInfo getUiShapeAt(QPoint position) const;
    QPoint uiPositionFromView(const QPoint& viewPosition) const;

    void updateUIFromConfig();

    // --- Setup ---
    void setupUiDownscaleTab();
    void setupUiPreprocessingTab();
    void setupUiInitTab();
    void setupUiAlgoTab();

    void setupConnections();

    void notifyConfigEdited();

    ImageView* settingsView_ = nullptr;
    InitializationBehavior* initializationBehavior_ = nullptr;

    ImageSettingsController* imageSettingsController_ = nullptr;

    // onglets a gauche
    QTabWidget* tabs_ = nullptr;

    // Ok Cancel en bas
    QDialogButtonBox* dial_buttons_ = nullptr;

    /////////////////////////////////////////
    //             onglets                 //
    /////////////////////////////////////////

    /////////////////////////////////////////

    QGroupBox* downscale_page_ = nullptr;
    QComboBox* downscale_factor_cb_ = nullptr;

    // widgets et variables liés à l'onglet preprocessing :

    QTabWidget* preprocess_tabs_ = nullptr;
    QGroupBox* preprocess_page_ = nullptr;

    // QCheckBox* is_downscale_cb;

    QGroupBox* gaussian_noise_groupbox_ = nullptr;
    QDoubleSpinBox* std_noise_spin_ = nullptr;

    QGroupBox* salt_noise_groupbox_ = nullptr;
    QDoubleSpinBox* salt_percent_spin_ = nullptr;

    QGroupBox* speckle_noise_groupbox_ = nullptr;
    QDoubleSpinBox* std_speckle_noise_spin_ = nullptr;

    QGroupBox* median_groupbox_ = nullptr;
    KernelSizeSpinBox* klength_median_spin_ = nullptr;
    QRadioButton* complex_sort_ = nullptr;
    QRadioButton* complex_perreault_ = nullptr;

    QGroupBox* mean_groupbox_ = nullptr;
    KernelSizeSpinBox* klength_mean_spin_ = nullptr;

    QGroupBox* gaussian_groupbox_ = nullptr;
    KernelSizeSpinBox* klength_gaussian_spin_ = nullptr;
    QDoubleSpinBox* std_filter_spin_ = nullptr;

    QGroupBox* aniso_groupbox_ = nullptr;
    QRadioButton* aniso1_radio_ = nullptr;
    QRadioButton* aniso2_radio_ = nullptr;
    QSpinBox* iteration_filter_spin_ = nullptr;
    QDoubleSpinBox* lambda_spin_ = nullptr;
    QDoubleSpinBox* kappa_spin_ = nullptr;

    QGroupBox* open_groupbox_ = nullptr;
    KernelSizeSpinBox* klength_open_spin_ = nullptr;

    QGroupBox* close_groupbox_ = nullptr;
    KernelSizeSpinBox* klength_close_spin_ = nullptr;

    QGroupBox* tophat_groupbox_ = nullptr;
    QRadioButton* whitetophat_radio_ = nullptr;
    QRadioButton* blacktophat_radio_ = nullptr;
    KernelSizeSpinBox* klength_tophat_spin_ = nullptr;

    QGroupBox* algo_groupbox_ = nullptr;
    QRadioButton* complex1_morpho_radio_ = nullptr;
    QRadioButton* complex2_morpho_radio_ = nullptr;

    QLabel* time_filt_ = nullptr;

    /////////////////////////////////////////

    // widgets et variables liés à l'onglet initialization :

    QWidget* init_page_ = nullptr;

    QRadioButton* rectangle_radio_ = nullptr;
    QRadioButton* ellipse_radio_ = nullptr;
    QSpinBox* width_shape_spin_ = nullptr;
    QSlider* width_slider_ = nullptr;
    QSpinBox* height_shape_spin_ = nullptr;
    QSlider* height_slider_ = nullptr;
    QSpinBox* abscissa_spin_ = nullptr;
    QSlider* abscissa_slider_ = nullptr;
    QSpinBox* ordinate_spin_ = nullptr;
    QSlider* ordinate_slider_ = nullptr;

    QPushButton* add_button_ = nullptr;
    QPushButton* subtract_button_ = nullptr;
    QPushButton* clear_button_ = nullptr;

    // widgets et variables liés à l'onglet algorithm :

    AlgoSettingsWidget* algo_widget_ = nullptr;
    QWidget* algo_page_ = nullptr;

    ImageSessionSettings config_;

    DownscaleConfig editedDownscaleConfig_;
    ProcessingConfig editedProcessingConfig_;

    int wheelAccumulator_ = 0;

private slots:

    void default_settings();
};

} // namespace fluvel_app
