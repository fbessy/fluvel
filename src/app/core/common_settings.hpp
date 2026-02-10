#ifndef COMMON_SETTINGS_HPP
#define COMMON_SETTINGS_HPP

#include "active_contour.hpp"
#include "region_color_ac.hpp"
#include "color.hpp"
#include "filters.hpp"
#include <QImage>


namespace ofeli_app
{

enum SpeedModel : int
{
    REGION_BASED = 0,  // Chan-Vese model
    EDGE_BASED         // Geodesic model
};

enum Language : int
{
    SYSTEM = 0,
    ENGLISH,
    FRENCH
};

enum ComboBoxColorIndex : int
{
    RED = 0,
    GREEN,
    BLUE,
    CYAN,
    MAGENTA,
    YELLOW,
    BLACK,
    WHITE,
    SELECTED,
};

enum class Session
{
    Image,
    Camera
};

struct AlgoConfig
{
    ofeli_ip::Connectivity connectivity;
    ofeli_ip::AcConfig ac_config;
    ofeli_ip::RegionColorConfig region_ac_config;
};

struct DownscaleConfig
{
    bool has_downscale;
    int downscale_factor;
};

struct DisplayConfig
{
    bool input_displayed;

    bool l_out_displayed;
    ofeli_ip::Rgb_uc l_out_color;

    bool l_in_displayed;
    ofeli_ip::Rgb_uc l_in_color;

    bool algorithm_overlay;
};

struct FilteringConfig
{
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
};

struct ImageSessionSettings
{
    AlgoConfig img_algo_conf;

    QImage initial_phi;

    bool has_preprocess;
    DownscaleConfig downscale_conf;
    FilteringConfig filtering_conf;

    DisplayConfig img_disp_conf;
};

struct CameraSessionSettings
{
    AlgoConfig cam_algo_conf;
    int cycles_nbr;

    DownscaleConfig downscale_conf;
    bool has_temporal_filtering;

    DisplayConfig cam_disp_conf;
    bool has_show_mirrored;
};

inline ofeli_ip::Rgb_uc get_color(int index)
{
    ofeli_ip::Rgb_uc color;

    switch( index )
    {
    case ComboBoxColorIndex::RED :
        color.red   = 255;
        color.green = 0;
        color.blue  = 0;
        break;

    case ComboBoxColorIndex::GREEN :
        color.red   = 0;
        color.green = 255;
        color.blue  = 0;
        break;

    case ComboBoxColorIndex::BLUE :
        color.red   = 0;
        color.green = 0;
        color.blue  = 255;
        break;

    case ComboBoxColorIndex::CYAN :
        color.red   = 0;
        color.green = 255;
        color.blue  = 255;
        break;

    case ComboBoxColorIndex::MAGENTA :
        color.red   = 255;
        color.green = 0;
        color.blue  = 255;
        break;

    case ComboBoxColorIndex::YELLOW :
        color.red   = 255;
        color.green = 255;
        color.blue  = 0;
        break;

    case ComboBoxColorIndex::BLACK :
        color.red   = 0;
        color.green = 0;
        color.blue  = 0;
        break;

    case ComboBoxColorIndex::WHITE :
        color.red   = 255;
        color.green = 255;
        color.blue  = 255;
        break;
    }

    return color;
}

inline int get_index(const ofeli_ip::Rgb_uc& color)
{
    int index = ComboBoxColorIndex::SELECTED;


    if ( color == ofeli_ip::Rgb_uc{255, 0, 0} )
    {
        index = ComboBoxColorIndex::RED;
    }

    else if ( color == ofeli_ip::Rgb_uc{0, 255, 0} )
    {
        index = ComboBoxColorIndex::GREEN;
    }

    else if ( color == ofeli_ip::Rgb_uc{0, 0, 255} )
    {
        index = ComboBoxColorIndex::BLUE;
    }

    else if ( color == ofeli_ip::Rgb_uc{0, 255, 255} )
    {
        index = ComboBoxColorIndex::CYAN;
    }

    else if ( color == ofeli_ip::Rgb_uc{255, 0, 255} )
    {
        index = ComboBoxColorIndex::MAGENTA;
    }

    else if ( color == ofeli_ip::Rgb_uc{255, 255, 0} )
    {
        index = ComboBoxColorIndex::YELLOW;
    }

    else if ( color == ofeli_ip::Rgb_uc{0, 0, 0} )
    {
        index = ComboBoxColorIndex::BLACK;
    }
    else if ( color == ofeli_ip::Rgb_uc{255, 255, 255} )
    {
        index = ComboBoxColorIndex::WHITE;
    }

    return index;
}

inline QRgb get_QRgb(ofeli_ip::Rgb_uc col)
{
    return qRgb(int(col.red),
                int(col.green),
                int(col.blue));
}

}

#endif // COMMON_SETTINGS_HPP
