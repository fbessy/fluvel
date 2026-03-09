// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "algo_settings_widget.hpp"
#include "filters.hpp"
#include "image_settings_controller.hpp"
#include "image_view.hpp"
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
    void onAddShape();
    void onSubtractShape();
    void onClearPhi();

    void onInputImageReady(const QImage& inputImage);

signals:
    void settingsAccepted(const ImageSessionSettings& config);

protected:
    //! Save the configuration chosen into the ApplicationSettings.
    void accept() override;

    //! Restore the ui states in function of the ApplicationSettings.
    void reject() override;

    void showEvent(QShowEvent* /*event*/) override;
    void closeEvent(QCloseEvent* event) override;

signals:
    void updateOverlay(fluvel_app::UiShapeInfo uiShape);

private:
    //////////////////////////////////////////
    //   pour la fenêtre de configuration   //
    /////////////////////////////////////////

    void onUiShapeChanged();
    UiShapeInfo getUiShape() const;

    void updateUIFromConfig();

    // --- Setup ---
    void setupUiDownscaleTab();
    void setupUiPreprocessingTab();
    void setupUiInitTab();
    void setupUiAlgoTab();

    void setupConnections();

    ImageView* settingsView_;

    ImageSettingsController* imageSettingsController_ = nullptr;

    // onglets a gauche
    QTabWidget* tabs_;

    // Ok Cancel en bas
    QDialogButtonBox* dial_buttons_;

    /////////////////////////////////////////
    //             onglets                 //
    /////////////////////////////////////////

    /////////////////////////////////////////

    QGroupBox* downscale_page_;
    QComboBox* downscale_factor_cb_;

    // widgets et variables liés à l'onglet preprocessing :

    QTabWidget* preprocess_tabs_;
    QGroupBox* preprocess_page_;

    // QCheckBox* is_downscale_cb;

    QVBoxLayout* noise_layout();

    QGroupBox* gaussian_noise_groupbox_;
    QDoubleSpinBox* std_noise_spin_;

    QGroupBox* salt_noise_groupbox_;
    QDoubleSpinBox* proba_noise_spin_;

    QGroupBox* speckle_noise_groupbox_;
    QDoubleSpinBox* std_speckle_noise_spin_;

    QVBoxLayout* filter_layout();

    QGroupBox* median_groupbox_;
    KernelSizeSpinBox* klength_median_spin_;
    QRadioButton* complex_radio1_;
    QRadioButton* complex_radio2_;

    QGroupBox* mean_groupbox_;
    KernelSizeSpinBox* klength_mean_spin_;

    QGroupBox* gaussian_groupbox_;
    KernelSizeSpinBox* klength_gaussian_spin_;
    QDoubleSpinBox* std_filter_spin_;

    QGroupBox* aniso_groupbox_;
    QRadioButton* aniso1_radio_;
    QRadioButton* aniso2_radio_;
    QSpinBox* iteration_filter_spin_;
    QDoubleSpinBox* lambda_spin_;
    QDoubleSpinBox* kappa_spin_;

    QGroupBox* open_groupbox_;
    KernelSizeSpinBox* klength_open_spin_;

    QGroupBox* close_groupbox_;
    KernelSizeSpinBox* klength_close_spin_;

    QGroupBox* tophat_groupbox_;
    QRadioButton* whitetophat_radio_;
    QRadioButton* blacktophat_radio_;
    KernelSizeSpinBox* klength_tophat_spin_;

    QGroupBox* algo_groupbox_;
    QRadioButton* complex1_morpho_radio_;
    QRadioButton* complex2_morpho_radio_;

    bool has_preprocess2_;
    QLabel* time_filt_;

    fluvel_ip::Filters* filters2_{nullptr};
    // float calculate_filtered_image();
    const unsigned char* img2_filtered_{nullptr};

    /////////////////////////////////////////

    // widgets et variables liés à l'onglet initialization :

    QWidget* init_page_;

    QRadioButton* rectangle_radio_;
    QRadioButton* ellipse_radio_;
    QSpinBox* width_shape_spin_;
    QSlider* width_slider_;
    QSpinBox* height_shape_spin_;
    QSlider* height_slider_;
    QSpinBox* abscissa_spin_;
    QSlider* abscissa_slider_;
    QSpinBox* ordinate_spin_;
    QSlider* ordinate_slider_;

    QPushButton* add_button_;
    QPushButton* subtract_button_;
    QPushButton* clear_button_;

    static unsigned char otsu_method(const int histogram[], unsigned int img_size);

    // widgets et variables liés à l'onglet algorithm :

    AlgoSettingsWidget* algo_widget_;
    QWidget* algo_page_;

    ImageSessionSettings config_;

private slots:

    // void do_scale0(int value);
    void default_settings();

    // slot appelé à chaque changement d'onglet
    // void tab_visu(int value);

    // slots appelés depuis l'onglet preprocessing
    // slot appelé aussi une fois dans l'onglet algorithm pour voir le gradient de l'image pour
    // le contour geodesique
    // void show_phi_with_filtered_image();

    // void change_display_size();
    // void set_color_out();
    // void set_color_in();
};

} // namespace fluvel_app
