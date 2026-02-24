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

enum class Language
{
    System = 0,
    English,
    French
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

enum class ImageBase
{
    Source,
    Preprocessed
};

struct DisplayConfig
{
    static constexpr ImageBase kDefaultImage = ImageBase::Preprocessed;

    static constexpr bool kDefaultListDisplayed = true;
    static constexpr unsigned char kDefaultRedOut   = 0u;
    static constexpr unsigned char kDefaultGreenOut = 0u;
    static constexpr unsigned char kDefaultBlueOut  = 255u;
    static constexpr unsigned char kDefaultRedIn    = 255u;
    static constexpr unsigned char kDefaultGreenIn  = 0u;
    static constexpr unsigned char kDefaultBlueIn   = 0u;

    static constexpr bool kDefaultOptions       = false;
    static constexpr bool kDefaultOverlay       = true;


    ImageBase image = kDefaultImage;

    bool l_out_displayed = kDefaultListDisplayed;
    ofeli_ip::Rgb_uc l_out_color { kDefaultRedOut,
                                   kDefaultGreenOut,
                                   kDefaultBlueOut };

    bool l_in_displayed = kDefaultListDisplayed;
    ofeli_ip::Rgb_uc l_in_color { kDefaultRedIn,
                                  kDefaultGreenIn,
                                  kDefaultBlueIn };

    bool mirrorMode        = kDefaultOptions;
    bool smoothDisplay     = kDefaultOptions;

    bool algorithm_overlay = kDefaultOverlay;
};

struct DownscaleConfig
{
    static constexpr bool kDefaultHasDownscale    = false;
    static constexpr int  kDefaultDownscaleFactor = 2;

    bool hasDownscale   = kDefaultHasDownscale;
    int downscaleFactor = kDefaultDownscaleFactor;
};

struct ProcessingConfig
{
    static constexpr bool  kDefaultProcess       = false;

    static constexpr float kDefaultStdNoise      = 20.f;
    static constexpr float kDefaultSaltNoise     = 0.05f;
    static constexpr float kDefaultSpeckleNoise  = 0.16f;

    static constexpr int   kDefaultKernelLength  = 5;
    static constexpr float kDefaultGaussianSigma = 2.f;

    static constexpr bool  kDefault01Algo        = true;

    static constexpr ofeli_ip::AnisoDiff kDefaultAnisoOption = ofeli_ip::AnisoDiff::FUNCTION1;
    static constexpr int   kDefaultMaxItera      = 10;
    static constexpr float kDefaultLambda        = 1.f/7.f;
    static constexpr float kDefaultKappa         = 30.f;

    static constexpr bool kDefaultWhiteTopHat    = true;

    bool has_gaussian_noise    = kDefaultProcess;
    float std_noise            = kDefaultStdNoise;

    bool has_salt_noise        = kDefaultProcess;
    float proba_noise          = kDefaultSaltNoise;

    bool has_speckle_noise     = kDefaultProcess;
    float std_speckle_noise    = kDefaultSpeckleNoise;

    bool has_median_filt       = kDefaultProcess;
    int kernel_median_length   = kDefaultKernelLength;
    bool has_O1_algo           = kDefault01Algo;

    bool has_mean_filt         = kDefaultProcess;
    int kernel_mean_length     = kDefaultKernelLength;

    bool has_gaussian_filt     = kDefaultProcess;
    int kernel_gaussian_length = kDefaultKernelLength;
    float sigma                = kDefaultGaussianSigma;

    bool has_aniso_diff              = kDefaultProcess;
    ofeli_ip::AnisoDiff aniso_option = kDefaultAnisoOption;
    int max_itera                    = kDefaultMaxItera;
    float lambda                     = kDefaultLambda;
    float kappa                      = kDefaultKappa;

    bool has_open_filt       = kDefaultProcess;
    int kernel_open_length   = kDefaultKernelLength;

    bool has_close_filt      = kDefaultProcess;
    int kernel_close_length  = kDefaultKernelLength;

    bool has_top_hat_filt    = kDefaultProcess;
    bool is_white_top_hat    = kDefaultWhiteTopHat;
    int kernel_tophat_length = kDefaultKernelLength;

    bool has_O1_morpho       = kDefault01Algo;

    bool hasProcessing() const
    {
        return has_gaussian_noise ||
               has_salt_noise ||
               has_speckle_noise ||
               has_median_filt ||
               has_mean_filt ||
               has_gaussian_filt ||
               has_aniso_diff ||
               has_open_filt ||
               has_close_filt ||
               has_top_hat_filt;
    }
};

struct AlgoConfig
{
    static constexpr ofeli_ip::Connectivity kDefaultConnectivity
        = ofeli_ip::Connectivity::Four;

    ofeli_ip::Connectivity connectivity;
    ofeli_ip::AcConfig acConfig;
    ofeli_ip::RegionColorConfig regionAcConfig;
};

struct ImageComputeConfig
{
    AlgoConfig algo;

    QImage initialPhi;

    DownscaleConfig downscale;
    ProcessingConfig processing;
};

struct ImageSessionSettings
{
    ImageComputeConfig compute;
    DisplayConfig display;
};

struct VideoComputeConfig
{
    static constexpr int  kDefaultCyclesNbr = 3;
    static constexpr bool kDefaultHasTemporalFiltering = true;

    AlgoConfig algo;
    int cyclesNbr = kDefaultCyclesNbr;

    DownscaleConfig downscale;
    bool hasTemporalFiltering = kDefaultHasTemporalFiltering;
};

struct VideoSessionSettings
{
    VideoComputeConfig compute;
    DisplayConfig display;
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
