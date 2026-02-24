/****************************************************************************
**
** Copyright (C) 2010-2025 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.
**
** http://www.cecill.info/licences/Licence_CeCILL_V2-en.html
** You may use this file under the terms of the CeCILL license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Fabien Bessy and its Subsidiary(-ies) nor the
**     names of its contributors may be used to endorse or promote products
**     derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

#ifndef APPLICATION_SETTINGS_HPP
#define APPLICATION_SETTINGS_HPP

#include "active_contour.hpp"
#include "region_color_ac.hpp"
#include "filters.hpp"
#include "color.hpp"
#include "common_settings.hpp"

#include <QObject>
#include <QDir>
#include <QImage>

namespace ofeli_app
{

//! This structure contains all the configuration of the application.
class ApplicationSettings : public QObject
{
    Q_OBJECT

public:

    ApplicationSettings();
    ~ApplicationSettings() override;

    void save();
    void save_img_session_config();
    void save_cam_session_config();

    void save_algo(Session session,
                   const AlgoConfig& algo);
    void save_downscale(Session session,
                        const DownscaleConfig& downscale_config);
    void save_disp(Session session,
                   const DisplayConfig& disp_config);

    void load_img_session_config();
    void load_cam_session_config();

    void load_algo(Session session,
                   AlgoConfig& algo);
    void load_downscale(Session session,
                        DownscaleConfig& downscale_config);
    void load_disp(Session session,
                   DisplayConfig& disp_config);

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
    void imgSettingsChanged(const ofeli_app::ImageSessionSettings& conf);
    void imgDisplaySettingsChanged(const ofeli_app::DisplayConfig& conf);
    void videoSettingsChanged(const ofeli_app::VideoSessionSettings& conf);
    void videoDisplaySettingsChanged(const ofeli_app::DisplayConfig& conf);
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

}

#endif // APPLICATION_SETTINGS_HPP
