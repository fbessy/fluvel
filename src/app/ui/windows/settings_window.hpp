/****************************************************************************
**
** Copyright (C) 2010-2025 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.
**
** http://www.cecill.info/licences/Licence_CeCILL_V2-en.html
** You may use this file under the terms of the CeCILL license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Fabien Bessy and its Subsidiary(-ies) nor the
**     names of its contributors may be used to endorse or promote products
**     derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

#ifndef SETTINGS_WINDOW_HPP
#define SETTINGS_WINDOW_HPP

#include "application_settings.hpp"
#include "filters.hpp"
#include "kernel_size_spinbox.hpp"
#include "grid2d.hpp"
#include "contour_rendering_qimage.hpp"
#include "contour_data.hpp"
#include "phi_editor.hpp"
#include "phi_view_model.hpp"
#include "image_view.hpp"
#include "algo_settings_widget.hpp"
#include "image_settings_controller.hpp"
#include "shape_type.hpp"

#include <QtWidgets>

namespace ofeli_app
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

public :

    SettingsWindow(QWidget* parent);

public slots:
    void onAddShape();
    void onSubtractShape();
    void onClearPhi();

    void onInputImageReady(const QImage& inputImage);

signals :
    void changed(const QMimeData* mimeData = 0);

protected:

    //! Save the configuration chosen into the ApplicationSettings.
    void accept() override;

    //! Restore the ui states in function of the ApplicationSettings.
    void reject() override;

    void showEvent(QShowEvent* /*event*/) override;
    void closeEvent(QCloseEvent* event) override;

signals:
    void updateOverlay(UiShapeInfo uiShape);

private :

    //////////////////////////////////////////
    //   pour la fenêtre de configuration   //
    /////////////////////////////////////////

    void onUiShapeChanged();
    UiShapeInfo getUiShape() const;

    void updateUIFromConfig();

    ImageView* settingsView;

    ImageSettingsController* imageSettingsController = nullptr;

    // onglets a gauche
    QTabWidget* tabs;

    // Ok Cancel en bas
    QDialogButtonBox* dial_buttons;

    /////////////////////////////////////////
    //             onglets                 //
    /////////////////////////////////////////

    /////////////////////////////////////////

    QGroupBox* downscale_page;
    QComboBox* downscale_factor_cb;

    // widgets et variables liés à l'onglet preprocessing :

    QTabWidget* preprocess_tabs;
    QGroupBox* preprocess_page;

    //QCheckBox* is_downscale_cb;

    QVBoxLayout* noise_layout();

    QGroupBox* gaussian_noise_groupbox;
    bool has_gaussian_noise2;
    QDoubleSpinBox* std_noise_spin;
    float std_noise2;

    QGroupBox* salt_noise_groupbox;
    bool has_salt_noise2;
    QDoubleSpinBox* proba_noise_spin;
    float proba_noise2;

    QGroupBox* speckle_noise_groupbox;
    bool has_speckle_noise2;
    QDoubleSpinBox* std_speckle_noise_spin;
    float std_speckle_noise2;

    QVBoxLayout* filter_layout();

    QGroupBox* median_groupbox;
    KernelSizeSpinBox* klength_median_spin;
    QRadioButton* complex_radio1;
    QRadioButton* complex_radio2;
    bool has_median_filt2;
    bool has_O1_algo2;
    int kernel_median_length2;

    QGroupBox* mean_groupbox;
    KernelSizeSpinBox* klength_mean_spin;
    bool has_mean_filt2;
    int kernel_mean_length2;

    QGroupBox* gaussian_groupbox;
    KernelSizeSpinBox* klength_gaussian_spin;
    QDoubleSpinBox* std_filter_spin;
    bool has_gaussian_filt2;
    int kernel_gaussian_length2;
    float sigma2;

    QGroupBox* aniso_groupbox;
    QRadioButton* aniso1_radio;
    QRadioButton* aniso2_radio;
    QSpinBox* iteration_filter_spin;
    QDoubleSpinBox* lambda_spin;
    QDoubleSpinBox* kappa_spin;
    bool has_aniso_diff2;
    int max_itera2;
    float lambda2;
    float kappa2;
    ofeli_ip::AnisoDiff aniso_option2;

    QGroupBox* open_groupbox;
    KernelSizeSpinBox* klength_open_spin;
    bool has_open_filt2;
    int kernel_open_length2;

    QGroupBox* close_groupbox;
    KernelSizeSpinBox* klength_close_spin;
    bool has_close_filt2;
    int kernel_close_length2;

    QGroupBox* tophat_groupbox;
    QRadioButton* whitetophat_radio;
    QRadioButton* blacktophat_radio;
    KernelSizeSpinBox* klength_tophat_spin;
    bool has_top_hat_filt2;
    bool is_white_top_hat2;
    int kernel_tophat_length2;

    QGroupBox* algo_groupbox;
    QRadioButton* complex1_morpho_radio;
    QRadioButton* complex2_morpho_radio;
    bool has_O1_morpho2;

    bool has_preprocess2;
    QLabel* time_filt;


    ofeli_ip::Filters* filters2;
    //float calculate_filtered_image();
    const unsigned char* img2_filtered;

    /////////////////////////////////////////

    // widgets et variables liés à l'onglet initialization :

    QWidget* init_page;

    void open_phi();
    void phiInit2imgPhi();
    void imgPhi2phiInit();

    QRadioButton* rectangle_radio;
    QRadioButton* ellipse_radio;
    QSpinBox* width_shape_spin;
    QSlider* width_slider;
    QSpinBox* height_shape_spin;
    QSlider* height_slider;
    QSpinBox* abscissa_spin;
    QSlider* abscissa_slider;
    QSpinBox* ordinate_spin;
    QSlider* ordinate_slider;

    QPushButton* add_button;
    QPushButton* subtract_button;
    QPushButton* clear_button;

    static unsigned char otsu_method(const int histogram[], unsigned int img_size);

    // widgets et variables liés à l'onglet algorithm :

    AlgoSettingsWidget* algo_widget;
    QWidget* algo_page;

    // --- Setup ---
    void setupUiDownscaleTab();
    void setupUiPreprocessingTab();
    void setupUiInitTab();
    void setupUiAlgoTab();

    void setupConnections();

private slots :

    //void do_scale0(int value);
    void default_settings();

    // slot appelé à chaque changement d'onglet
    //void tab_visu(int value);

    // slots appelés depuis l'onglet preprocessing
    // slot appelé aussi une fois dans l'onglet algorithm pour voir le gradient de l'image pour le contour geodesique
    //void show_phi_with_filtered_image();

    //void change_display_size();
    //void set_color_out();
    //void set_color_in();
};

}

#endif // SETTINGS_WINDOW_HPP
