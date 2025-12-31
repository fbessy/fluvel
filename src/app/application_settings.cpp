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

#include "application_settings.hpp"
#include "boundary_builder.hpp"
#include "filters.hpp"

#include <QSettings>

namespace ofeli_gui
{

ApplicationSettings::ApplicationSettings()
{
    QSettings settings;

    app_language = Language( settings.value("Language/current_index",
                                             Language::SYSTEM).toInt() );

    //////////////////////////////////////////////////
    // Algorithm

    algo_config.is_cycle2 = settings.value("Settings/Algorithm/has_smoothing_cycle", true).toBool();
    algo_config.kernel_length = settings.value("Settings/Algorithm/kernel_curve", 5).toInt();
    algo_config.sigma = settings.value("Settings/Algorithm/std_curve", 2.f).toFloat();
    algo_config.Na = settings.value("Settings/Algorithm/Na", 30).toInt();
    algo_config.Ns = settings.value("Settings/Algorithm/Ns", 3).toInt();

    speed = SpeedModel( settings.value("Settings/Algorithm/speed", SpeedModel::REGION_BASED).toUInt() );

    region_ac_config.lambda_in = settings.value("Settings/Algorithm/lambda_in", 1).toInt();
    region_ac_config.lambda_out = settings.value("Settings/Algorithm/lambda_out", 1).toInt();

    region_ac_config.color_space = ofeli_ip::ColorSpaceOption(settings.value("Settings/Algorithm/color_space", ofeli_ip::ColorSpaceOption::RGB).toUInt());

    region_ac_config.weights[0] = settings.value("Settings/Algorithm/alpha", 1).toInt();
    region_ac_config.weights[1] = settings.value("Settings/Algorithm/beta", 1).toInt();
    region_ac_config.weights[2] = settings.value("Settings/Algorithm/gamma", 1).toInt();

    kernel_gradient_length = settings.value("Settings/Algorithm/kernel_gradient_length", 5).toInt();

    downscale_factor = settings.value("Settings/Algorithm/downscale_factor", 4).toUInt();
    cycles_nbr = settings.value("Settings/Algorithm/cycles_nbr", 3).toUInt();


    //////////////////////////////////////////////////
    // Initialization

    phi_width = settings.value("Settings/Initialization/phi_width", 100).toInt();
    phi_height = settings.value("Settings/Initialization/phi_height", 100).toInt();
    int Lout_init_size = settings.beginReadArray("Settings/Initialization/Lout_init");
    for( int index = Lout_init_size-1; index >= 0; index-- )
    {
        settings.setArrayIndex(index);
        Lout_init.push_front( settings.value("Offset").toInt() );
    }
    settings.endArray();

    int Lin_init_size = settings.beginReadArray("Settings/Initialization/Lin_init");
    for( int index = Lin_init_size-1; index >= 0; index-- )
    {
        settings.setArrayIndex(index);
        Lin_init.push_front( settings.value("Offset").toInt() );
    }
    settings.endArray();

    if( Lout_init.empty() || Lin_init.empty() )
    {
        Lout_init.clear();
        Lin_init.clear();

        ofeli_ip::BoundaryBuilder lists_init( phi_width,
                                              phi_height,
                                              Lout_init,
                                              Lin_init );

        lists_init.get_rectangle_points(0.05f, 0.05f, 0.95f, 0.95f);
    }

    has_ellipse = settings.value("Settings/Initialization/has_ellipse", true).toBool();
    init_width = settings.value("Settings/Initialization/init_width", 0.65f).toFloat();
    init_height = settings.value("Settings/Initialization/init_height", 0.65f).toFloat();
    center_x = settings.value("Settings/Initialization/center_x", 0.f).toFloat();
    center_y = settings.value("Settings/Initialization/center_y", 0.f).toFloat();

    //////////////////////////////////////////////////
    // Preprocessing

    has_preprocess = settings.value("Settings/Preprocessing/has_preprocess", false).toBool();

    has_gaussian_noise = settings.value("Settings/Preprocessing/has_gaussian_noise", false).toBool();
    std_noise = settings.value("Settings/Preprocessing/std_noise", 20.f).toFloat();
    has_salt_noise = settings.value("Settings/Preprocessing/has_salt_noise", false).toBool();
    proba_noise = settings.value("Settings/Preprocessing/proba_noise", 0.05f).toFloat();
    has_speckle_noise = settings.value("Settings/Preprocessing/has_speckle_noise", false).toBool();
    std_speckle_noise = settings.value("Settings/Preprocessing/std_speckle_noise", 0.16f).toFloat();

    has_median_filt = settings.value("Settings/Preprocessing/has_median_filt", false).toBool();
    kernel_median_length = settings.value("Settings/Preprocessing/kernel_median_length", 5).toInt();
    has_O1_algo = settings.value("Settings/Preprocessing/has_O1_algo", true).toBool();
    has_mean_filt = settings.value("Settings/Preprocessing/has_mean_filt", false).toBool();
    kernel_mean_length = settings.value("Settings/Preprocessing/kernel_mean_length", 5).toInt();
    has_gaussian_filt = settings.value("Settings/Preprocessing/has_gaussian_filt", false).toBool();
    kernel_gaussian_length = settings.value("Settings/Preprocessing/kernel_gaussian_length", 5).toInt();
    sigma = settings.value("Settings/Preprocessing/sigma", 2.f).toFloat();

    has_aniso_diff = settings.value("Settings/Preprocessing/has_aniso_diff", false).toBool();
    aniso_option = ofeli_ip::AnisoDiff( settings.value("Settings/Preprocessing/aniso_option", ofeli_ip::AnisoDiff::FUNCTION1).toUInt() );
    max_itera = settings.value("Settings/Preprocessing/max_itera", 10).toInt();
    lambda = settings.value("Settings/Preprocessing/lambda", 1.f/7.f).toFloat();
    kappa = settings.value("Settings/Preprocessing/kappa", 30.f).toFloat();

    has_open_filt = settings.value("Settings/Preprocessing/has_open_filt", false).toBool();
    kernel_open_length = settings.value("Settings/Preprocessing/kernel_open_length", 5).toInt();
    has_close_filt = settings.value("Settings/Preprocessing/has_close_filt", false).toBool();
    kernel_close_length = settings.value("Settings/Preprocessing/kernel_close_length", 5).toInt();
    has_top_hat_filt = settings.value("Settings/Preprocessing/has_top_hat_filt", false).toBool();
    is_white_top_hat = settings.value("Settings/Preprocessing/is_white_top_hat", true).toBool();
    kernel_tophat_length = settings.value("Settings/Preprocessing/kernel_tophat_length", 5).toInt();

    has_O1_morpho = settings.value("Settings/Preprocessing/has_O1_morpho", true).toBool();

    ////////////////////////////////////////////
    // Display

    has_histo_normaliz = settings.value("Settings/Display/has_histo_normaliz", true).toBool();

    has_display_each = settings.value("Settings/Display/has_display_each", true).toBool();

    outside_combo = settings.value("Settings/Display/outside_combo", QComboBoxColorIndex::BLUE).toInt();
    inside_combo = settings.value("Settings/Display/inside_combo", QComboBoxColorIndex::RED).toInt();

    selected_out.red   = (unsigned char)(settings.value("Settings/Display/Rout_selected", 128).toUInt());
    selected_out.green = (unsigned char)(settings.value("Settings/Display/Gout_selected", 0).toUInt());
    selected_out.blue  = (unsigned char)(settings.value("Settings/Display/Bout_selected", 255).toUInt());
    selected_in.red    = (unsigned char)(settings.value("Settings/Display/Rin_selected", 255).toUInt());
    selected_in.green  = (unsigned char)(settings.value("Settings/Display/Gin_selected", 128).toUInt());
    selected_in.blue   = (unsigned char)(settings.value("Settings/Display/Bin_selected", 0).toUInt());

    if( outside_combo == QComboBoxColorIndex::SELECTED )
    {
        color_out = selected_out;
    }
    else
    {
        get_color( outside_combo, color_out );
    }

    if( inside_combo == QComboBoxColorIndex::SELECTED )
    {
        color_in = selected_in;
    }
    else
    {
        get_color( inside_combo, color_in );
    }

    is_show_fps = settings.value("Settings/Display/is_show_fps", true).toBool();
    is_show_mirrored = settings.value("Settings/Display/is_show_mirrored", true).toBool();
}

ApplicationSettings::~ApplicationSettings()
{
    Lout_init.clear();
    Lin_init.clear();
}


void ApplicationSettings::save()
{
    QSettings settings;

    settings.setValue("Language/current_index", app_language);

    settings.setValue("Settings/Algorithm/Na", algo_config.Na);
    settings.setValue("Settings/Algorithm/Ns", algo_config.Ns);
    settings.setValue("Settings/Algorithm/speed", speed);
    settings.setValue("Settings/Algorithm/lambda_out", region_ac_config.lambda_out);
    settings.setValue("Settings/Algorithm/lambda_in", region_ac_config.lambda_in);
    settings.setValue("Settings/Algorithm/kernel_gradient_length", kernel_gradient_length);
    settings.setValue("Settings/Algorithm/color_space", region_ac_config.color_space);
    settings.setValue("Settings/Algorithm/alpha", region_ac_config.weights[0]);
    settings.setValue("Settings/Algorithm/beta", region_ac_config.weights[1]);
    settings.setValue("Settings/Algorithm/gamma", region_ac_config.weights[2]);
    settings.setValue("Settings/Algorithm/has_smoothing_cycle", algo_config.is_cycle2);
    settings.setValue("Settings/Algorithm/kernel_curve", algo_config.kernel_length);
    settings.setValue("Settings/Algorithm/std_curve", algo_config.sigma);

    settings.setValue("Settings/Algorithm/downscale_factor", downscale_factor);
    settings.setValue("Settings/Algorithm/cycles_nbr", cycles_nbr);

    settings.setValue("Settings/Initialization/phi_width", phi_width);
    settings.setValue("Settings/Initialization/phi_height", phi_height);

    settings.remove("Settings/Initialization/Lout_init");
    settings.beginWriteArray("Settings/Initialization/Lout_init");
    int index = 0;
    for( auto it = Lout_init.cbegin();
         it != Lout_init.cend();
         ++it )
    {
        settings.setArrayIndex(index);
        settings.setValue( "Offset", *it );

        index++;
    }
    settings.endArray();

    settings.remove("Settings/Initialization/Lin_init");
    settings.beginWriteArray("Settings/Initialization/Lin_init");
    index = 0;
    for( auto it = Lin_init.cbegin();
         it != Lin_init.cend();
         ++it )
    {
        settings.setArrayIndex(index);
        settings.setValue( "Offset", *it );

        index++;
    }
    settings.endArray();

    settings.setValue("Settings/Initialization/has_ellipse", has_ellipse);
    settings.setValue("Settings/Initialization/init_width", init_width);
    settings.setValue("Settings/Initialization/init_height", init_height);
    settings.setValue("Settings/Initialization/center_x", center_x);
    settings.setValue("Settings/Initialization/center_y", center_y);

    settings.setValue("Settings/Preprocessing/has_preprocess", has_preprocess);
    settings.setValue("Settings/Preprocessing/has_gaussian_noise", has_gaussian_noise);
    settings.setValue("Settings/Preprocessing/std_noise", std_noise);
    settings.setValue("Settings/Preprocessing/has_salt_noise", has_salt_noise);
    settings.setValue("Settings/Preprocessing/proba_noise", proba_noise);
    settings.setValue("Settings/Preprocessing/has_speckle_noise", has_speckle_noise);
    settings.setValue("Settings/Preprocessing/std_speckle_noise", std_speckle_noise);

    settings.setValue("Settings/Preprocessing/has_median_filt", has_median_filt);
    settings.setValue("Settings/Preprocessing/kernel_median_length", kernel_median_length);
    settings.setValue("Settings/Preprocessing/has_O1_algo", has_O1_algo);
    settings.setValue("Settings/Preprocessing/has_mean_filt", has_mean_filt);
    settings.setValue("Settings/Preprocessing/kernel_mean_length", kernel_mean_length);
    settings.setValue("Settings/Preprocessing/has_gaussian_filt", has_gaussian_filt);
    settings.setValue("Settings/Preprocessing/kernel_gaussian_length", kernel_gaussian_length);
    settings.setValue("Settings/Preprocessing/sigma", sigma);

    settings.setValue("Settings/Preprocessing/has_aniso_diff", has_aniso_diff);
    settings.setValue("Settings/Preprocessing/aniso_option", aniso_option);
    settings.setValue("Settings/Preprocessing/max_itera", max_itera);
    settings.setValue("Settings/Preprocessing/lambda", lambda);
    settings.setValue("Settings/Preprocessing/kappa", kappa);

    settings.setValue("Settings/Preprocessing/has_open_filt", has_open_filt);
    settings.setValue("Settings/Preprocessing/kernel_open_length", kernel_open_length);

    settings.setValue("Settings/Preprocessing/has_close_filt", has_close_filt);
    settings.setValue("Settings/Preprocessing/kernel_close_length", kernel_close_length);

    settings.setValue("Settings/Preprocessing/has_top_hat_filt", has_top_hat_filt);
    settings.setValue("Settings/Preprocessing/is_white_top_hat", is_white_top_hat);
    settings.setValue("Settings/Preprocessing/kernel_tophat_length", kernel_tophat_length);

    settings.setValue("Settings/Preprocessing/has_O1_morpho", has_O1_morpho);
    settings.setValue("Settings/Display/has_histo_normaliz", has_histo_normaliz);
    settings.setValue("Settings/Display/has_display_each", has_display_each);

    settings.setValue("Settings/Display/inside_combo", inside_combo);
    settings.setValue("Settings/Display/outside_combo", outside_combo);

    settings.setValue("Settings/Display/Rout_selected", (unsigned int)(selected_out.red));
    settings.setValue("Settings/Display/Gout_selected", (unsigned int)(selected_out.green));
    settings.setValue("Settings/Display/Bout_selected", (unsigned int)(selected_out.blue));
    settings.setValue("Settings/Display/Rin_selected", (unsigned int)(selected_in.red));
    settings.setValue("Settings/Display/Gin_selected", (unsigned int)(selected_in.green));
    settings.setValue("Settings/Display/Bin_selected", (unsigned int)(selected_in.blue));

    settings.setValue("Settings/Display/is_show_fps", is_show_fps);
    settings.setValue("Settings/Display/is_show_mirrored", is_show_mirrored);
}

}
