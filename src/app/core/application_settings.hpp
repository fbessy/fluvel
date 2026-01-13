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
#include "runtime_settings.hpp"

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
    virtual ~ApplicationSettings();

    void save();
    RuntimeSettings snapshot() const;

    QDir settingsDirectory();

    bool save_initial_phi();
    void load_default_initial_phi();
    bool load_initial_phi();

    Language app_language;

    ofeli_ip::AcConfig algo_config;
    ofeli_ip::RegionColorConfig region_ac_config;

    SpeedModel speed;
    int kernel_gradient_length;

    unsigned int downscale_factor;
    unsigned int cycles_nbr;

    /////////////////////////////////////////

    bool has_ellipse;
    float init_width;
    float init_height;
    float center_x;
    float center_y;

    QImage initialPhi;

    /////////////////////////////////////////

    bool has_preprocess;

    bool has_gaussian_noise;
    float std_noise;
    bool has_salt_noise;
    float proba_noise;
    bool has_speckle_noise;
    float std_speckle_noise;

    bool has_median_filt;
    int kernel_median_length;
    bool has_O1_algo;
    bool has_mean_filt;
    int kernel_mean_length;
    bool has_gaussian_filt;
    int kernel_gaussian_length;
    float sigma;

    bool has_aniso_diff;
    ofeli_ip::AnisoDiff aniso_option;
    int max_itera;
    float lambda;
    float kappa;

    bool has_open_filt;
    int kernel_open_length;
    bool has_close_filt;
    int kernel_close_length;
    bool has_top_hat_filt;
    bool is_white_top_hat;
    int kernel_tophat_length;

    bool has_O1_morpho;

    /////////////////////////////////////////

    bool has_histo_normaliz;

    bool has_display_each;

    int outside_combo;
    int inside_combo;

    RgbColor color_out;
    RgbColor color_in;
    RgbColor selected_out;
    RgbColor selected_in;

    bool is_show_fps;
    bool is_show_mirrored;

signals:
    void settingsApplied();
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

}

#endif // APPLICATION_SETTINGS_HPP
