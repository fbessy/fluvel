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
#include "pixmap_widget.hpp"
#include "scroll_area_widget.hpp"
#include "kernel_size_spinbox.hpp"
#include "matrix.hpp"
#include "contour_rendering.hpp"
#include "contour_data.hpp"
#include "phi_editor.hpp"
#include "phi_view_model.hpp"

#include <QtWidgets>

namespace ofeli_app
{

enum TabIndex : int
{
    ALGORITHM = 0,
    INITIALIZATION,
    PREPROCESSING,
    DISPLAY
};

class SettingsWindow : public QDialog
{

    Q_OBJECT

public :

    SettingsWindow(QWidget* parent);

    const unsigned char* get_filtered_img_data();

protected:

    //! Save the configuration chosen into the ApplicationSettings.
    void accept() override;

    //! Restore the ui states in function of the ApplicationSettings.
    void reject() override;

private :

    virtual void closeEvent(QCloseEvent* event) override;

    //////////////////////////////////////////
    //   pour la fenêtre de configuration   //
    /////////////////////////////////////////

    // image dans un scrollarea a droite dans la fenêtre
    PixmapWidget* imageLabel_settings;
    ScrollAreaWidget* scrollArea_settings;

    // onglets a gauche
    QTabWidget* tabs;

    // Ok Cancel en bas
    QDialogButtonBox* dial_buttons;

    /////////////////////////////////////////
    //             onglets                 //
    /////////////////////////////////////////

    // widgets et variables liés à l'onglet algorithm :

    QSpinBox* Na_spin;
    QSpinBox* Ns_spin;
    QRadioButton* chanvese_radio;
    QSpinBox* lambda_out_spin;
    QSpinBox* lambda_in_spin;
    QRadioButton* geodesic_radio;
    KernelSizeSpinBox* klength_gradient_spin;
    QGroupBox* color_weights_groupbox;
    QComboBox* color_space_cb;
    QSpinBox* alpha_spin;
    QSpinBox* beta_spin;
    QSpinBox* gamma_spin;
    SpeedModel speed;
    int kernel_gradient_length2;
    unsigned int alpha2;
    unsigned int beta2;
    unsigned int gamma2;

    QGroupBox* internalspeed_groupbox;
    KernelSizeSpinBox* klength_spin;
    QDoubleSpinBox* std_spin;

    QComboBox* downscale_factor_cb;
    QSpinBox* cycles_nbr_sb;


    /////////////////////////////////////////

    // widgets et variables liés à l'onglet initialization :

    QWidget* page2;

    QPushButton* open_phi_button;
    QPushButton* save_phi_button;

    //QImage img_phi;
    void open_phi();
    void phiInit2imgPhi();
    void imgPhi2phiInit();

    QRadioButton* rectangle_radio;
    QRadioButton* ellipse_radio;
    bool has_ellipse2;

    QSpinBox* width_shape_spin;
    QSlider* width_slider;
    float init_width2;
    QSpinBox* height_shape_spin;
    QSlider* height_slider;
    float init_height2;

    QSpinBox* abscissa_spin;
    QSlider* abscissa_slider;
    float center_x2;
    QSpinBox* ordinate_spin;
    QSlider* ordinate_slider;
    float center_y2;

    QPushButton* add_button;
    QPushButton* subtract_button;
    QPushButton* clean_button;

    static unsigned char otsu_method(const int histogram[], unsigned int img_size);

    /////////////////////////////////////////

    // widgets et variables liés à l'onglet preprocessing :

    QGroupBox* preprocessing();

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

    QTabWidget* preprocess_tabs;
    QGroupBox* page3;
    bool has_preprocess2;
    QLabel* time_filt;


    /////////////////////////////////////////

    // widgets et variables liés à l'onglet display :

    QSpinBox* scale_spin;
    QSlider* scale_slider;
    QCheckBox* histo_checkbox;
    bool has_histo_normaliz2;


    QCheckBox* step_checkbox;
    QComboBox* outsidecolor_combobox;

    // colors to display contour in settings window
    // as a preview color settings before user acceptance
    RgbColor color_out_disp;
    RgbColor selected_out_disp;
    RgbColor color_in_disp;
    RgbColor selected_in_disp;

    QComboBox* insidecolor_combobox;

    // for group box tracking
    QCheckBox* fps_checkbox;
    QCheckBox* mirrored_checkbox;


    ofeli_ip::Filters* filters2;
    float calculate_filtered_image();
    void calculate_filtered_copy_visu_buffers();
    const unsigned char* img2_filtered;

    // phi avant nettoyage des frontières
    ofeli_ip::Matrix<signed char>* phi2;
    std::vector<ofeli_ip::ContourPoint> Lout33;
    std::vector<ofeli_ip::ContourPoint> Lin33;

    ofeli_ip::Matrix<signed char>* displayed_phi_shape;
    std::vector<ofeli_ip::ContourPoint> Lout_shape11;
    std::vector<ofeli_ip::ContourPoint> Lin_shape11;

    void do_flood_fill_from_lists(const std::vector<ofeli_ip::ContourPoint>& Lout, const std::vector<ofeli_ip::ContourPoint>& Lin,
                                  ofeli_ip::Matrix<signed char>& phi);

    void do_flood_fill(ofeli_ip::Matrix<signed char>& phi, int offset_seed,
                       ofeli_ip::PhiValue target_value, ofeli_ip::PhiValue replacement_value);

    void find_lists_from_phi(const ofeli_ip::Matrix<signed char>& phi,
                             std::vector<ofeli_ip::ContourPoint>& Lout,
                             std::vector<ofeli_ip::ContourPoint>& Lin);

    bool find_redundant_list_point(const ofeli_ip::Matrix<signed char>& phi, int offset) const;

    QImage img;

    QImage image_filter;
    QImage image_phi;
    QImage image_shape;

    QString last_directory_used;

    QPixmap pixmap_settings;

    void phi_add_shape();
    void phi_subtract_shape();
    void phi_visu(bool dark_color);

    /////////////////////////////////////////////////
    //        variables liés au slot open()        //
    /////////////////////////////////////////////////

    // chaîne de caractère du chemin+nom de l'image obtenu a partir d'une boite de dialogue ouverture de Qt
    QString fileName_phi;
    // buffer et informations des images
    // buffer 8 bits
    const unsigned char* img1;
    bool is_rgb1;
    // ligne
    int img_width;
    // colonne
    int img_height;

    // taille de l'image
    int img_size;
    int find_offset(int x, int y) const;

    bool eventFilter(QObject* object, QEvent* event);
    // position du curseur souris
    int positionX;
    int positionY;
    void show_phi_list_value();

    // pour la fenêtre de configuration des parametres
    void mouse_move_event_settings(QMouseEvent* event);
    void mouse_press_event_settings(QMouseEvent* event);

    bool has_contours_hidden;
    void img1_visu();
    bool has_show_img1;
    void update_visu();

    void drag_enter_event_phi(QDragEnterEvent* event);
    void drag_move_event_phi(QDragMoveEvent* event);
    void drop_event_phi(QDropEvent* event);
    void drag_leave_event_phi(QDragLeaveEvent* event);

    void save_phi_without_given_extension(const QString& img_str,
                                          const QString& selected_filter,
                                          const QString& fileName_save,
                                          const QImage& img_phi_save);

    QStringList nameFilters;


    std::unique_ptr<PhiEditor> phiEditor;
    std::unique_ptr<PhiViewModel> phiViewModel;

private slots :

    //void do_scale0(int value);
    void default_settings();

    // slot appelé à chaque changement d'onglet
    void tab_visu(int value);

    // slots appelés depuis l'onglet initialization
    void openFilenamePhi();
    void save_phi();
    void clean_phi_visu();
    void shape_visu(int value);
    void phi_visu(int value);
    void add_visu();
    void subtract_visu();
    void shape_visu();

    // slots appelés depuis l'onglet preprocessing
    // slot appelé aussi une fois dans l'onglet algorithm pour voir le gradient de l'image pour le contour geodesique
    void show_phi_with_filtered_image();

    // slots appelés depuis l'onglet Display
    void do_scale(int value);
    void change_display_size();
    void set_color_out();
    void set_color_in();

    void adjustVerticalScroll_settings(int min, int max);
    void adjustHorizontalScroll_settings(int min, int max);

    void wheel_zoom(int,ScrollAreaWidget*);

signals :

    void changed(const QMimeData* mimeData = 0);

public slots:
    void init2(const QImage& img);
};

inline int SettingsWindow::find_offset(int x, int y) const
{
    return x+y*img_width;
}

}

#endif // SETTINGS_WINDOW_HPP
