// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "common_settings.hpp"

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
    ApplicationSettings();
    ~ApplicationSettings() override = default;

    void save();

    void setInitialPhiImage(const QImage& phi);
    const ImageSessionSettings& imageSettings() const;
    const VideoSessionSettings& videoSettings() const;

    Language appLanguage() const;
    void setAppLanguage(Language language);

    void resizeInitialPhiImage(int width, int height);

signals:
    void imgSettingsChanged(const fluvel_app::ImageSessionSettings& conf);
    void imgDisplaySettingsChanged(const fluvel_app::DisplayConfig& conf);
    void videoSettingsChanged(const fluvel_app::VideoSessionSettings& conf);
    void videoDisplaySettingsChanged(const fluvel_app::DisplayConfig& conf);

    void resizedPhi(const QImage& phi);

public slots:
    void updateImageSessionSettings(const ImageSessionSettings& config);
    void updateVideoSessionSettings(const VideoSessionSettings& config);
    void setImageDisplayConfig(const DisplayConfig& displayConfig);
    void setVideoDisplayConfig(const DisplayConfig& displayConfig);

private:
    void loadImageSessionSettings();
    void loadVideoSessionSettings();
    void saveImageSessionSettings();
    void saveVideoSessionSettings();

    void loadAlgo(Session session, AlgoConfig& algoConfig);
    void loadDownscale(Session session, DownscaleConfig& downscaleConfig);
    void loadDisplay(Session session, DisplayConfig& displayConfig);

    void saveAlgo(Session session, const AlgoConfig& algoConfig);
    void saveDownscale(Session session, const DownscaleConfig& downscaleConfig);
    void saveDisplay(Session session, const DisplayConfig& displayConfig);

    QDir settingsDirectory();

    void loadDefaultInitialPhi();
    bool loadInitialPhi();
    bool saveInitialPhi();

    ImageSessionSettings imageSettings_;
    VideoSessionSettings videoSettings_;

    Language appLanguage_;
};

class AppSettings
{
public:
    static ApplicationSettings& instance()
    {
        static ApplicationSettings settings;
        return settings;
    }
};

QString toSettingsPrefix(Session scope);

} // namespace fluvel_app
