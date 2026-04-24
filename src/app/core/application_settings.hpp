// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "application_settings_types.hpp"

#include <QObject>
#include <QImage>
#include <QDir>

namespace fluvel_app
{

//! This structure contains all the configuration of the application.
class ApplicationSettings : public QObject
{
    Q_OBJECT

public:
    static ApplicationSettings& instance();

    ApplicationSettings();
    ~ApplicationSettings() override = default;

    void save();
    void saveQuiet();

    const ImageSessionSettings& imageSettings() const;
    const VideoSessionSettings& videoSettings() const;

    Language appLanguage() const;
    void setAppLanguage(Language language);

    void setImageSessionSettings(const ImageSessionSettings& config);
    void setVideoSessionSettings(const VideoSessionSettings& config);
    void setImageDisplayConfig(const DisplayConfig& displayConfig);
    void setVideoDisplayConfig(const DisplayConfig& displayConfig);

signals:
    void imgSettingsChanged(const fluvel_app::ImageSessionSettings& conf);
    void imgDisplaySettingsChanged(const fluvel_app::DisplayConfig& conf);
    void videoSettingsChanged(const fluvel_app::VideoSessionSettings& conf);
    void videoDisplaySettingsChanged(const fluvel_app::DisplayConfig& conf);

private:
    void loadImageSessionSettings();
    void loadVideoSessionSettings();
    void saveImageSessionSettings();
    void saveVideoSessionSettings();

    void loadAlgo(Session session, ActiveContourConfig& algoConfig);
    void loadDownscale(Session session, DownscaleParams& downscaleParams);
    void loadDisplay(Session session, DisplayConfig& displayConfig);

    void saveAlgo(Session session, const ActiveContourConfig& algoConfig);
    void saveDownscale(Session session, const DownscaleParams& downscaleParams);
    void saveDisplay(Session session, const DisplayConfig& displayConfig);

    QDir settingsDirectory();

    void loadDefaultInitialPhi();
    bool loadInitialPhi();
    bool saveInitialPhi();

    ImageSessionSettings imageSettings_;
    VideoSessionSettings videoSettings_;

    Language appLanguage_;
};

} // namespace fluvel_app
