#ifndef RUNTIME_SETTINGS_HPP
#define RUNTIME_SETTINGS_HPP

#include "common_settings.hpp"
#include "active_contour.hpp"
#include "region_color_ac.hpp"
#include "filters.hpp"

#include <QImage>

namespace ofeli_app
{

struct RuntimeSettings
{
    Language app_language;

    ofeli_ip::AcConfig algo_config;
    ofeli_ip::RegionColorConfig region_ac_config;

    SpeedModel speed;
    int kernel_gradient_length;

    unsigned int downscale_factor;
    unsigned int cycles_nbr;

    /////////////////////////////////////////

    bool has_ellipse;
    float init_width;
    float init_height;
    float center_x;
    float center_y;

    QImage initialPhi;

    /////////////////////////////////////////

    bool has_preprocess;

    bool has_gaussian_noise;
    float std_noise;
    bool has_salt_noise;
    float proba_noise;
    bool has_speckle_noise;
    float std_speckle_noise;

    bool has_median_filt;
    int kernel_median_length;
    bool has_O1_algo;
    bool has_mean_filt;
    int kernel_mean_length;
    bool has_gaussian_filt;
    int kernel_gaussian_length;
    float sigma;

    bool has_aniso_diff;
    ofeli_ip::AnisoDiff aniso_option;
    int max_itera;
    float lambda;
    float kappa;

    bool has_open_filt;
    int kernel_open_length;
    bool has_close_filt;
    int kernel_close_length;
    bool has_top_hat_filt;
    bool is_white_top_hat;
    int kernel_tophat_length;

    bool has_O1_morpho;

    /////////////////////////////////////////

    bool has_histo_normaliz;

    bool has_display_each;

    int outside_combo;
    int inside_combo;

    RgbColor color_out;
    RgbColor color_in;
    RgbColor selected_out;
    RgbColor selected_in;

    bool is_show_fps;
    bool is_show_mirrored;
};

}

#endif // RUNTIME_SETTINGS_HPP
