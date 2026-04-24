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

inline constexpr const char* to_string(ImageDisplayMode mode)
{
    switch (mode)
    {
        case ImageDisplayMode::Source:
            return "source";
        case ImageDisplayMode::Preprocessed:
            return "preprocessed";
    }
    return "source";
}

inline ImageDisplayMode imageDisplayModeFromString(std::string_view s)
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

    static constexpr bool kDefaultContourVisible = true;
    static constexpr fluvel_ip::Rgb_uc kDefaultOuterContourColor{64u, 0u, 255u};
    static constexpr fluvel_ip::Rgb_uc kDefaultInnerContourColor{0u, 230u, 118u};

    static constexpr bool kDefaultOptionDisabled = false;
    static constexpr bool kDefaultOverlayEnabled = true;

    ImageDisplayMode displayMode = kDefaultDisplayMode;

    bool outerContourVisible = kDefaultContourVisible;
    fluvel_ip::Rgb_uc outerContourColor{kDefaultOuterContourColor};

    bool innerContourVisible = kDefaultContourVisible;
    fluvel_ip::Rgb_uc innerContourColor{kDefaultInnerContourColor};

    bool mirrorMode = kDefaultOptionDisabled;
    bool smoothDisplay = kDefaultOptionDisabled;

    bool algorithmOverlayEnabled = kDefaultOverlayEnabled;
};

struct DownscaleParams
{
    static constexpr bool kDefaultDownscaleEnabled = false;
    static constexpr int kDefaultDownscaleFactor = 2;

    bool downscaleEnabled = kDefaultDownscaleEnabled;
    int downscaleFactor = kDefaultDownscaleFactor;
};

struct ActiveContourConfig
{
    static constexpr fluvel_ip::Connectivity kDefaultConnectivity = fluvel_ip::Connectivity::Four;

    fluvel_ip::Connectivity connectivity = kDefaultConnectivity;
    fluvel_ip::ActiveContourParams contourParams{};
    fluvel_ip::RegionColorParams regionParams{};
};

struct ImageComputeConfig
{
    ActiveContourConfig contourConfig{};

    QImage initialPhi;

    DownscaleParams downscale{};
    fluvel_ip::ProcessingParams processing{};
};

struct ImageSessionSettings
{
    ImageComputeConfig compute{};
    DisplayConfig display{};
};

struct VideoComputeConfig
{
    static constexpr bool kDefaultSpatialFilteringEnabled = true;
    static constexpr bool kDefaultTemporalFilteringEnabled = true;

    DownscaleParams downscale{};
    bool spatialFilteringEnabled = kDefaultSpatialFilteringEnabled;
    bool temporalFilteringEnabled = kDefaultTemporalFilteringEnabled;

    ActiveContourConfig contourConfig{};
};

struct VideoSessionSettings
{
    VideoComputeConfig compute{};
    DisplayConfig display{};
};

} // namespace fluvel_app
