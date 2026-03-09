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
    void save_img_session_config();
    void save_img_session_config_with_val(const ImageSessionSettings& config);

    void save_cam_session_config();
    void save_cam_session_config_with_val(const VideoSessionSettings& config);

    void save_algo(Session session, const AlgoConfig& algo);
    void save_downscale(Session session, const DownscaleConfig& downscale_config);
    void save_disp(Session session, const DisplayConfig& disp_config);

    void load_img_session_config();
    void load_cam_session_config();

    void load_algo(Session session, AlgoConfig& algo);
    void load_downscale(Session session, DownscaleConfig& downscale_config);
    void load_disp(Session session, DisplayConfig& disp_config);

    void set_img_display_config(const DisplayConfig& disp_config);
    void set_cam_display_config(const DisplayConfig& disp_config);

    QDir settingsDirectory();

    bool save_initial_phi();
    void load_default_initial_phi();
    bool load_initial_phi();
    void resize_initial_phi(int width, int height);

    Language app_language;
    ImageSessionSettings imgConfig;
    VideoSessionSettings camConfig;

signals:
    void imgSettingsChanged(const fluvel_app::ImageSessionSettings& conf);
    void imgDisplaySettingsChanged(const fluvel_app::DisplayConfig& conf);
    void videoSettingsChanged(const fluvel_app::VideoSessionSettings& conf);
    void videoDisplaySettingsChanged(const fluvel_app::DisplayConfig& conf);
    void resizedPhi(const QImage& phi);
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
