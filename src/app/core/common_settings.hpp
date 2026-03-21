// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ac_types.hpp"
#include "color.hpp"
#include "filters.hpp"

#include <QImage>
#include <string_view>

namespace fluvel_app
{

enum class Language
{
    System = 0,
    English,
    French
};

inline constexpr const char* to_string(Language lang)
{
    switch (lang)
    {
        case Language::System:
            return "system";
        case Language::English:
            return "en";
        case Language::French:
            return "fr";
    }
    return "system";
}

inline Language language_from_string(std::string_view s)
{
    if (s == "en")
        return Language::English;
    if (s == "fr")
        return Language::French;
    return Language::System;
}

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

inline constexpr const char* to_string(ImageBase ib)
{
    switch (ib)
    {
        case ImageBase::Source:
            return "source";
        case ImageBase::Preprocessed:
            return "preprocessed";
    }
    return "source";
}

inline ImageBase ib_from_string(std::string_view s)
{
    if (s == "source")
        return ImageBase::Source;
    if (s == "preprocessed")
        return ImageBase::Preprocessed;
    return ImageBase::Source;
}

struct DisplayConfig
{
    static constexpr ImageBase kDefaultImageBase = ImageBase::Preprocessed;

    static constexpr bool kDefaultListDisplayed = true;
    static constexpr fluvel_ip::Rgb_uc kDefaultOut{64u, 0u, 255u};
    static constexpr fluvel_ip::Rgb_uc kDefaultIn{0u, 230u, 118u};

    static constexpr bool kDefaultOptions = false;
    static constexpr bool kDefaultOverlay = true;

    ImageBase image = kDefaultImageBase;

    bool l_out_displayed = kDefaultListDisplayed;
    fluvel_ip::Rgb_uc l_out_color{kDefaultOut};

    bool l_in_displayed = kDefaultListDisplayed;
    fluvel_ip::Rgb_uc l_in_color{kDefaultIn};

    bool mirrorMode = kDefaultOptions;
    bool smoothDisplay = kDefaultOptions;

    bool algorithm_overlay = kDefaultOverlay;
};

struct DownscaleConfig
{
    static constexpr bool kDefaultHasDownscale = false;
    static constexpr int kDefaultDownscaleFactor = 2;

    bool hasDownscale = kDefaultHasDownscale;
    int downscaleFactor = kDefaultDownscaleFactor;
};

struct ProcessingConfig
{
    static constexpr bool kDefaultProcess = false;

    static constexpr float kDefaultStdNoise = 20.f;
    static constexpr float kDefaultSaltNoise = 0.05f;
    static constexpr float kDefaultSpeckleNoise = 0.16f;

    static constexpr int kDefaultKernelLength = 5;
    static constexpr float kDefaultGaussianSigma = 2.f;

    static constexpr bool kDefault01Algo = true;

    static constexpr fluvel_ip::AnisoDiff kDefaultAnisoOption = fluvel_ip::AnisoDiff::FUNCTION1;
    static constexpr int kDefaultMaxItera = 10;
    static constexpr float kDefaultLambda = 1.f / 7.f;
    static constexpr float kDefaultKappa = 30.f;

    static constexpr bool kDefaultWhiteTopHat = true;

    bool enabled = false;

    bool has_gaussian_noise = kDefaultProcess;
    float std_noise = kDefaultStdNoise;

    bool has_salt_noise = kDefaultProcess;
    float proba_noise = kDefaultSaltNoise;

    bool has_speckle_noise = kDefaultProcess;
    float std_speckle_noise = kDefaultSpeckleNoise;

    bool has_median_filt = kDefaultProcess;
    int kernel_median_length = kDefaultKernelLength;
    bool has_O1_algo = kDefault01Algo;

    bool has_mean_filt = kDefaultProcess;
    int kernel_mean_length = kDefaultKernelLength;

    bool has_gaussian_filt = kDefaultProcess;
    int kernel_gaussian_length = kDefaultKernelLength;
    float sigma = kDefaultGaussianSigma;

    bool has_aniso_diff = kDefaultProcess;
    fluvel_ip::AnisoDiff aniso_option = kDefaultAnisoOption;
    int max_itera = kDefaultMaxItera;
    float lambda = kDefaultLambda;
    float kappa = kDefaultKappa;

    bool has_open_filt = kDefaultProcess;
    int kernel_open_length = kDefaultKernelLength;

    bool has_close_filt = kDefaultProcess;
    int kernel_close_length = kDefaultKernelLength;

    bool has_top_hat_filt = kDefaultProcess;
    bool is_white_top_hat = kDefaultWhiteTopHat;
    int kernel_tophat_length = kDefaultKernelLength;

    bool has_O1_morpho = kDefault01Algo;

    bool hasProcessing() const
    {
        return enabled && (has_gaussian_noise || has_salt_noise || has_speckle_noise ||
                           has_median_filt || has_mean_filt || has_gaussian_filt ||
                           has_aniso_diff || has_open_filt || has_close_filt || has_top_hat_filt);
    }
};

struct AlgoConfig
{
    static constexpr fluvel_ip::Connectivity kDefaultConnectivity = fluvel_ip::Connectivity::Four;

    fluvel_ip::Connectivity connectivity;
    fluvel_ip::AcConfig acConfig;
    fluvel_ip::RegionColorConfig regionAcConfig;
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
    static constexpr bool kDefaultUseOptimizedFormat = true;
    static constexpr bool kDefaultHasTemporalFiltering = true;

    bool useOptimizedFormat = kDefaultUseOptimizedFormat;
    DownscaleConfig downscale;
    bool hasTemporalFiltering = kDefaultHasTemporalFiltering;

    AlgoConfig algo;
};

struct VideoSessionSettings
{
    VideoComputeConfig compute;
    DisplayConfig display;
};

inline fluvel_ip::Rgb_uc get_color(int index)
{
    fluvel_ip::Rgb_uc color;

    switch (index)
    {
        case ComboBoxColorIndex::RED:
            color.red = 255;
            color.green = 0;
            color.blue = 0;
            break;

        case ComboBoxColorIndex::GREEN:
            color.red = 0;
            color.green = 255;
            color.blue = 0;
            break;

        case ComboBoxColorIndex::BLUE:
            color.red = 0;
            color.green = 0;
            color.blue = 255;
            break;

        case ComboBoxColorIndex::CYAN:
            color.red = 0;
            color.green = 255;
            color.blue = 255;
            break;

        case ComboBoxColorIndex::MAGENTA:
            color.red = 255;
            color.green = 0;
            color.blue = 255;
            break;

        case ComboBoxColorIndex::YELLOW:
            color.red = 255;
            color.green = 255;
            color.blue = 0;
            break;

        case ComboBoxColorIndex::BLACK:
            color.red = 0;
            color.green = 0;
            color.blue = 0;
            break;

        case ComboBoxColorIndex::WHITE:
            color.red = 255;
            color.green = 255;
            color.blue = 255;
            break;
    }

    return color;
}

inline int get_index(const fluvel_ip::Rgb_uc& color)
{
    int index = ComboBoxColorIndex::SELECTED;

    if (color == fluvel_ip::Rgb_uc{255, 0, 0})
    {
        index = ComboBoxColorIndex::RED;
    }

    else if (color == fluvel_ip::Rgb_uc{0, 255, 0})
    {
        index = ComboBoxColorIndex::GREEN;
    }

    else if (color == fluvel_ip::Rgb_uc{0, 0, 255})
    {
        index = ComboBoxColorIndex::BLUE;
    }

    else if (color == fluvel_ip::Rgb_uc{0, 255, 255})
    {
        index = ComboBoxColorIndex::CYAN;
    }

    else if (color == fluvel_ip::Rgb_uc{255, 0, 255})
    {
        index = ComboBoxColorIndex::MAGENTA;
    }

    else if (color == fluvel_ip::Rgb_uc{255, 255, 0})
    {
        index = ComboBoxColorIndex::YELLOW;
    }

    else if (color == fluvel_ip::Rgb_uc{0, 0, 0})
    {
        index = ComboBoxColorIndex::BLACK;
    }
    else if (color == fluvel_ip::Rgb_uc{255, 255, 255})
    {
        index = ComboBoxColorIndex::WHITE;
    }

    return index;
}

inline QRgb get_QRgb(fluvel_ip::Rgb_uc col)
{
    return qRgb(int(col.red), int(col.green), int(col.blue));
}

} // namespace fluvel_app
