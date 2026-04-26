// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file application_settings.hpp
 * @brief Persistent application-wide settings management.
 *
 * This module provides a singleton responsible for storing, loading, and
 * updating all application settings, including image and video session
 * configurations as well as display preferences.
 *
 * Settings are persisted on disk and propagated to the UI via Qt signals.
 */

#pragma once

#ifndef Q_MOC_RUN
#include "application_settings_types.hpp"
#endif

#include <QObject>
#include <QImage>
#include <QDir>

namespace fluvel_app
{

//! This structure contains all the configuration of the application.
/**
 * @brief Singleton managing application settings.
 *
 * This class centralizes all configuration used by the application,
 * including:
 * - Image session settings
 * - Video session settings
 * - Display configuration
 * - Application language
 *
 * It provides persistence (load/save) and notifies the UI of changes
 * through Qt signals.
 *
 * @note Thread affinity follows Qt object rules. Intended for use in the main thread.
 */
class ApplicationSettings : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Access the global ApplicationSettings instance.
     *      * @return Singleton instance.
     */
    static ApplicationSettings& instance();

    /**
     * @brief Construct the settings manager.
     *      * Typically initializes settings from persistent storage.
     */
    ApplicationSettings();
    ~ApplicationSettings() override = default;

    /**
     * @brief Save all settings to persistent storage.
     */
    void save();

    /**
     * @brief Save settings without emitting signals or UI updates.
     */
    void saveQuiet();

    /**
     * @brief Get image session settings.
     */
    const ImageSessionSettings& imageSettings() const;

    /**
     * @brief Get video session settings.
     */
    const VideoSessionSettings& videoSettings() const;

    /**
     * @brief Get current application language.
     */
    Language appLanguage() const;

    /**
     * @brief Set application language.
     */
    void setAppLanguage(Language language);

    /**
     * @brief Update full image session settings.
     */
    void setImageSessionSettings(const ImageSessionSettings& config);

    /**
     * @brief Update full video session settings.
     */
    void setVideoSessionSettings(const VideoSessionSettings& config);

    /**
     * @brief Update full video session settings.
     */
    void setImageDisplayConfig(const DisplayConfig& displayConfig);

    /**
     * @brief Update video display configuration only.
     */
    void setVideoDisplayConfig(const DisplayConfig& displayConfig);

signals:
    /**
     * @brief Emitted when image session settings change.
     */
    void imgSettingsChanged(const fluvel_app::ImageSessionSettings& conf);

    /**
     * @brief Emitted when image display settings change.
     */
    void imgDisplaySettingsChanged(const fluvel_app::DisplayConfig& conf);

    /**
     * @brief Emitted when video session settings change.
     */
    void videoSettingsChanged(const fluvel_app::VideoSessionSettings& conf);

    /**
     * @brief Emitted when video display settings change.
     */
    void videoDisplaySettingsChanged(const fluvel_app::DisplayConfig& conf);

private:
    /// Load image session settings from storage.
    void loadImageSessionSettings();

    /// Load video session settings from storage.
    void loadVideoSessionSettings();

    /// Save image session settings to storage.
    void saveImageSessionSettings();

    /// Save video session settings to storage.
    void saveVideoSessionSettings();

    /// Load algorithm configuration for a given session.
    void loadAlgo(Session session, ActiveContourConfig& algoConfig);

    /// Load downscale parameters for a given session.
    void loadDownscale(Session session, DownscaleParams& downscaleParams);

    /// Load display configuration for a given session.
    void loadDisplay(Session session, DisplayConfig& displayConfig);

    /// Save algorithm configuration for a given session.
    void saveAlgo(Session session, const ActiveContourConfig& algoConfig);

    /// Save downscale parameters for a given session.
    void saveDownscale(Session session, const DownscaleParams& downscaleParams);

    /// Save display configuration for a given session.
    void saveDisplay(Session session, const DisplayConfig& displayConfig);

    /**
     * @brief Get the directory where settings are stored.
     */
    QDir settingsDirectory();

    /// Load default initial level-set function.
    void loadDefaultInitialPhi();

    /// Load initial level-set from storage.
    bool loadInitialPhi();

    /// Save initial level-set to storage.
    bool saveInitialPhi();

    /// Current image session settings.
    ImageSessionSettings imageSettings_;

    /// Current video session settings.
    VideoSessionSettings videoSettings_;

    /// Current application language.
    Language appLanguage_;
};

} // namespace fluvel_app
