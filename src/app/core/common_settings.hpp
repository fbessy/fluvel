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
    static constexpr ImageBase defaultImage = ImageBase::Preprocessed;

    static constexpr bool defaultListDisplayed = true;
    static constexpr unsigned char defaultRedOut   = 0u;
    static constexpr unsigned char defaultGreenOut = 0u;
    static constexpr unsigned char defaultBlueOut  = 255u;
    static constexpr unsigned char defaultRedIn    = 255u;
    static constexpr unsigned char defaultGreenIn  = 0u;
    static constexpr unsigned char defaultBlueIn   = 0u;

    static constexpr bool defaultOptions       = false;
    static constexpr bool defaultOverlay       = true;


    ImageBase image = defaultImage;

    bool l_out_displayed = defaultListDisplayed;
    ofeli_ip::Rgb_uc l_out_color { defaultRedOut,
                                   defaultGreenOut,
                                   defaultBlueOut };

    bool l_in_displayed = defaultListDisplayed;
    ofeli_ip::Rgb_uc l_in_color { defaultRedIn,
                                  defaultGreenIn,
                                  defaultBlueIn };

    bool mirrorMode        = defaultOptions;
    bool smoothDisplay   = defaultOptions;

    bool algorithm_overlay = defaultOverlay;
};

struct DownscaleConfig
{
    static constexpr bool defaultHasDownscale    = false;
    static constexpr int  defaultDownscaleFactor = 2;

    bool hasDownscale   = defaultHasDownscale;
    int downscaleFactor = defaultDownscaleFactor;
};

struct ProcessingConfig
{
    static constexpr bool  defaultProcess       = false;

    static constexpr float defaultStdNoise      = 20.f;
    static constexpr float defaultSaltNoise     = 0.05f;
    static constexpr float defaultSpeckleNoise  = 0.16f;

    static constexpr int   defaultKernelLength  = 5;
    static constexpr float defaultGaussianSigma = 2.f;

    static constexpr bool  default01Algo        = true;

    static constexpr ofeli_ip::AnisoDiff defaultAnisoOption = ofeli_ip::AnisoDiff::FUNCTION1;
    static constexpr int   defaultMaxItera      = 10;
    static constexpr float defaultLambda        = 1.f/7.f;
    static constexpr float defaultKappa         = 30.f;

    static constexpr bool defaultWhiteTopHat    = true;

    bool has_gaussian_noise    = defaultProcess;
    float std_noise            = defaultStdNoise;

    bool has_salt_noise        = defaultProcess;
    float proba_noise          = defaultSaltNoise;

    bool has_speckle_noise     = defaultProcess;
    float std_speckle_noise    = defaultSpeckleNoise;

    bool has_median_filt       = defaultProcess;
    int kernel_median_length   = defaultKernelLength;
    bool has_O1_algo           = default01Algo;

    bool has_mean_filt         = defaultProcess;
    int kernel_mean_length     = defaultKernelLength;

    bool has_gaussian_filt     = defaultProcess;
    int kernel_gaussian_length = defaultKernelLength;
    float sigma                = defaultGaussianSigma;

    bool has_aniso_diff              = defaultProcess;
    ofeli_ip::AnisoDiff aniso_option = defaultAnisoOption;
    int max_itera                    = defaultMaxItera;
    float lambda                     = defaultLambda;
    float kappa                      = defaultKappa;

    bool has_open_filt       = defaultProcess;
    int kernel_open_length   = defaultKernelLength;

    bool has_close_filt      = defaultProcess;
    int kernel_close_length  = defaultKernelLength;

    bool has_top_hat_filt    = defaultProcess;
    bool is_white_top_hat    = defaultWhiteTopHat;
    int kernel_tophat_length = defaultKernelLength;

    bool has_O1_morpho       = default01Algo;

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
    static constexpr ofeli_ip::Connectivity defaultConnectivity
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
    static constexpr int  defaultCyclesNbr = 3;
    static constexpr bool defaultHasTemporalFiltering = true;

    AlgoConfig algo;
    int cyclesNbr = defaultCyclesNbr;

    DownscaleConfig downscale;
    bool hasTemporalFiltering = defaultHasTemporalFiltering;
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
