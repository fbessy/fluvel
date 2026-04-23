// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour_types.hpp"
#include "color.hpp"
#include "contour_types.hpp"
#include "image_pipeline.hpp"
#include "speed_model_types.hpp"

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

enum class ImageDisplayMode
{
    Source,
    Preprocessed
};

inline constexpr const char* to_string(ImageDisplayMode ib)
{
    switch (ib)
    {
        case ImageDisplayMode::Source:
            return "source";
        case ImageDisplayMode::Preprocessed:
            return "preprocessed";
    }
    return "source";
}

inline ImageDisplayMode ib_from_string(std::string_view s)
{
    if (s == "source")
        return ImageDisplayMode::Source;
    if (s == "preprocessed")
        return ImageDisplayMode::Preprocessed;
    return ImageDisplayMode::Source;
}

struct DisplayConfig
{
    static constexpr ImageDisplayMode kDefaultDisplayMode = ImageDisplayMode::Preprocessed;

    static constexpr bool kDefaultListDisplayed = true;
    static constexpr fluvel_ip::Rgb_uc kDefaultOut{64u, 0u, 255u};
    static constexpr fluvel_ip::Rgb_uc kDefaultIn{0u, 230u, 118u};

    static constexpr bool kDefaultOptions = false;
    static constexpr bool kDefaultOverlay = true;

    ImageDisplayMode mode = kDefaultDisplayMode;

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

struct AlgoConfig
{
    static constexpr fluvel_ip::Connectivity kDefaultConnectivity = fluvel_ip::Connectivity::Four;

    fluvel_ip::Connectivity connectivity;
    fluvel_ip::ActiveContourParams acConfig;
    fluvel_ip::RegionColorParams regionAcConfig;
};

struct ImageComputeConfig
{
    AlgoConfig algo;

    QImage initialPhi;

    DownscaleConfig downscale;
    fluvel_ip::ProcessingConfig processing;
};

struct ImageSessionSettings
{
    ImageComputeConfig compute;
    DisplayConfig display;
};

struct VideoComputeConfig
{
    static constexpr bool kDefaultHasSpatialFiltering = true;
    static constexpr bool kDefaultHasTemporalFiltering = true;

    DownscaleConfig downscale;
    bool hasSpatialFiltering = kDefaultHasSpatialFiltering;
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
