// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file application_settings_types.hpp
 * @brief Application-level configuration types for Fluvel.
 *
 * This module defines configuration structures used by the application layer
 * (fluvel_app), including display settings, processing parameters, and session
 * configurations for both image and video workflows.
 *
 * These types act as a bridge between UI controls and the core processing layer.
 */

#pragma once

#include "active_contour_types.hpp"
#include "color.hpp"
#include "contour_types.hpp"
#include "processing_params.hpp"
#include "speed_model_types.hpp"

#include <QImage>
#include <string_view>

namespace fluvel_app
{

/**
 * @brief Application language selection.
 */
enum class Language
{
    System,  ///< Use system locale
    English, ///< English language
    French   ///< French language
};

/**
 * @brief Convert language enum to string identifier.
 *
 * @param lang Language value.
 * @return Corresponding string ("system", "en", "fr").
 */
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

/**
 * @brief Convert string identifier to Language enum.
 *
 * @param s Input string.
 * @return Corresponding Language value (defaults to System).
 */
inline Language language_from_string(std::string_view s)
{
    if (s == "en")
        return Language::English;
    if (s == "fr")
        return Language::French;
    return Language::System;
}

/**
 * @brief Session type.
 *
 * Defines the current processing context.
 */
enum class Session
{
    Image, ///< Static image processing
    Camera ///< Live camera processing
};

/**
 * @brief Image display mode.
 *
 * Controls which image representation is shown in the UI.
 */
enum class ImageDisplayMode
{
    Source,      ///< Original input image
    Preprocessed ///< Processed image after pipeline
};

/**
 * @brief Convert display mode to string.
 */
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

/**
 * @brief Convert string to display mode.
 */
inline ImageDisplayMode imageDisplayModeFromString(std::string_view s)
{
    if (s == "source")
        return ImageDisplayMode::Source;
    if (s == "preprocessed")
        return ImageDisplayMode::Preprocessed;
    return ImageDisplayMode::Source;
}

/**
 * @brief Display configuration for rendering images and contours.
 *
 * Controls visualization options such as contour visibility, colors,
 * and rendering effects.
 */
struct DisplayConfig
{
    static constexpr ImageDisplayMode kDefaultDisplayMode{ImageDisplayMode::Preprocessed};

    static constexpr bool kDefaultContourVisible{true};
    static constexpr fluvel_ip::Rgb_uc kDefaultOuterContourColor{64u, 0u, 255u};
    static constexpr fluvel_ip::Rgb_uc kDefaultInnerContourColor{0u, 230u, 118u};

    static constexpr bool kDefaultOptionDisabled{false};
    static constexpr bool kDefaultOverlayEnabled{true};

    ImageDisplayMode displayMode = kDefaultDisplayMode;

    bool outerContourVisible = kDefaultContourVisible;
    fluvel_ip::Rgb_uc outerContourColor{kDefaultOuterContourColor};

    bool innerContourVisible = kDefaultContourVisible;
    fluvel_ip::Rgb_uc innerContourColor{kDefaultInnerContourColor};

    bool mirrorMode = kDefaultOptionDisabled;
    bool smoothDisplay = kDefaultOptionDisabled;

    bool algorithmOverlayEnabled = kDefaultOverlayEnabled;
};

/**
 * @brief Parameters controlling optional image downscaling.
 *
 * Used to reduce computation cost by processing a lower-resolution image.
 */
struct DownscaleParams
{
    static constexpr bool kDefaultDownscaleEnabled{false};
    static constexpr int kDefaultDownscaleFactor{2};

    bool downscaleEnabled = kDefaultDownscaleEnabled;
    int downscaleFactor = kDefaultDownscaleFactor;
};

/**
 * @brief Parameters controlling optional image downscaling.
 *
 * Used to reduce computation cost by processing a lower-resolution image.
 */
struct ActiveContourConfig
{
    static constexpr fluvel_ip::Connectivity kDefaultConnectivity{fluvel_ip::Connectivity::Four};

    fluvel_ip::Connectivity connectivity = kDefaultConnectivity;
    fluvel_ip::ActiveContourParams contourParams{};
    fluvel_ip::RegionColorParams regionParams{};
};

/**
 * @brief Configuration for image processing pipeline.
 *
 * Combines active contour parameters, preprocessing, and optional downscaling.
 */
struct ImageComputeConfig
{
    ActiveContourConfig contourConfig{};

    QImage initialPhi;

    DownscaleParams downscale{};
    fluvel_ip::ProcessingParams processing{};
};

/**
 * @brief Complete settings for an image session.
 *
 * Includes both computation and display configuration.
 */
struct ImageSessionSettings
{
    ImageComputeConfig compute{};
    DisplayConfig display{};
};

/**
 * @brief Complete settings for an image session.
 *
 * Includes both computation and display configuration.
 */
struct VideoComputeConfig
{
    static constexpr bool kDefaultSpatialFilteringEnabled{true};
    static constexpr bool kDefaultTemporalFilteringEnabled{true};

    DownscaleParams downscale{};
    bool spatialFilteringEnabled = kDefaultSpatialFilteringEnabled;
    bool temporalFilteringEnabled = kDefaultTemporalFilteringEnabled;

    ActiveContourConfig contourConfig{};

    VideoComputeConfig()
    {
        // Video mode: prefer robustness over strict failure stopping
        contourConfig.contourParams.failureMode = fluvel_ip::FailureHandlingMode::RecoverOnFailure;
    }
};

/**
 * @brief Complete settings for a video session.
 *
 * Includes both computation and display configuration.
 */
struct VideoSessionSettings
{
    VideoComputeConfig compute{};
    DisplayConfig display{};
};

} // namespace fluvel_app
