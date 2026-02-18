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
#include "contour_rendering_qimage.hpp"

#include <QSettings>
#include <QPainter>

namespace ofeli_app
{

namespace {
static constexpr const char* kPhiInitFilename = "phi_init.png";
}

ApplicationSettings::ApplicationSettings()
{
    QSettings settings;

    app_language = Language(
        settings.value("Language/current",
                       int(Language::System)).toInt()
        );

    load_img_session_config();
    load_cam_session_config();
}

ApplicationSettings::~ApplicationSettings()
{
}


void ApplicationSettings::save()
{
    QSettings settings;

    settings.setValue("Language/current", int(app_language));

    save_img_session_config();
    save_cam_session_config();
}

void ApplicationSettings::save_img_session_config()
{
    QSettings settings;

    // algo

    save_algo("img",
              imgConfig.compute.algo);

    // phi init used only for image session
    save_initial_phi();

    save_downscale("img",
                   imgConfig.compute.downscale);

    settings.setValue("img/preprocess/has_gaussian_noise",
                      imgConfig.compute.processing.has_gaussian_noise);
    settings.setValue("img/preprocess/std_noise",
                      imgConfig.compute.processing.std_noise);
    settings.setValue("img/preprocess/has_salt_noise",
                      imgConfig.compute.processing.has_salt_noise);
    settings.setValue("img/preprocess/proba_noise",
                      imgConfig.compute.processing.proba_noise);
    settings.setValue("img/preprocess/has_speckle_noise",
                      imgConfig.compute.processing.has_speckle_noise);
    settings.setValue("img/preprocess/std_speckle_noise",
                      imgConfig.compute.processing.std_speckle_noise);

    settings.setValue("img/preprocess/has_median_filt",
                      imgConfig.compute.processing.has_median_filt);
    settings.setValue("img/preprocess/kernel_median_length",
                      imgConfig.compute.processing.kernel_median_length);

    settings.setValue("img/preprocess/has_O1_algo",
                      imgConfig.compute.processing.has_O1_algo);
    settings.setValue("img/preprocess/has_mean_filt",
                      imgConfig.compute.processing.has_mean_filt);
    settings.setValue("img/preprocess/kernel_mean_length",
                      imgConfig.compute.processing.kernel_mean_length);
    settings.setValue("img/preprocess/has_gaussian_filt",
                      imgConfig.compute.processing.has_gaussian_filt);
    settings.setValue("img/preprocess/kernel_gaussian_length",
                      imgConfig.compute.processing.kernel_gaussian_length);
    settings.setValue("img/preprocess/sigma",
                      imgConfig.compute.processing.sigma);

    settings.setValue("img/preprocess/has_aniso_diff",
                      imgConfig.compute.processing.has_aniso_diff);
    settings.setValue("img/preprocess/aniso_option",
                      imgConfig.compute.processing.aniso_option);
    settings.setValue("img/preprocess/max_itera",
                      imgConfig.compute.processing.max_itera);
    settings.setValue("img/Preprocessing/lambda",
                      imgConfig.compute.processing.lambda);
    settings.setValue("img/Preprocessing/kappa",
                      imgConfig.compute.processing.kappa);

    settings.setValue("img/preprocess/has_open_filt",
                      imgConfig.compute.processing.has_open_filt);
    settings.setValue("img/preprocess/kernel_open_length",
                      imgConfig.compute.processing.kernel_open_length);

    settings.setValue("img/preprocess/has_close_filt",
                      imgConfig.compute.processing.has_close_filt);
    settings.setValue("img/preprocess/kernel_close_length",
                      imgConfig.compute.processing.kernel_close_length);

    settings.setValue("img/preprocess/has_top_hat_filt",
                      imgConfig.compute.processing.has_top_hat_filt);
    settings.setValue("img/preprocess/is_white_top_hat",
                      imgConfig.compute.processing.is_white_top_hat);
    settings.setValue("img/preprocess/kernel_tophat_length",
                      imgConfig.compute.processing.kernel_tophat_length);

    settings.setValue("img/preprocess/has_O1_morpho",
                      imgConfig.compute.processing.has_O1_morpho);


    // display

    save_disp("img",
              imgConfig.display);

    emit imgSettingsChanged(imgConfig);
}

void ApplicationSettings::save_cam_session_config()
{
    QSettings settings;

    // algo

    save_algo("cam",
              camConfig.compute.algo);

    settings.setValue("cam/algo/cyclesNbr",
                      camConfig.compute.cyclesNbr);

    // preprocess

    save_downscale("cam",
                   camConfig.compute.downscale);

    settings.setValue("cam/preprocess/hasTemporalFiltering",
                      camConfig.compute.hasTemporalFiltering);

    // display

    save_disp("cam",
              camConfig.display);

    emit camSettingsChanged(camConfig);
}

void ApplicationSettings::save_algo(const QString& scope,
                                    const AlgoConfig& algo_config)
{
    QSettings settings;

    settings.setValue(scope + "/algo/connectivity",
                      int(algo_config.connectivity));

    settings.setValue(scope + "/algo/Na",
                      algo_config.acConfig.Na);

    settings.setValue(scope + "/algo/has_smoothing_cycle",
                      algo_config.acConfig.is_cycle2);

    settings.setValue(scope + "/algo/Ns",
                      algo_config.acConfig.Ns);

    settings.setValue(scope + "/algo/disk_radius",
                      algo_config.acConfig.disk_radius);

    settings.setValue(scope + "/algo/lambda_out",
                      algo_config.regionAcConfig.lambda_out);

    settings.setValue(scope + "/algo/lambda_in",
                      algo_config.regionAcConfig.lambda_in);

    settings.setValue(scope + "/algo/color_space",
                      int(algo_config.regionAcConfig.color_space));

    settings.setValue(scope + "/algo/alpha",
                      algo_config.regionAcConfig.weights.c1);

    settings.setValue(scope + "/algo/beta",
                      algo_config.regionAcConfig.weights.c2);

    settings.setValue(scope + "/algo/gamma",
                      algo_config.regionAcConfig.weights.c3);
}

void ApplicationSettings::save_downscale(const QString& scope,
                                         const DownscaleConfig& downscale_config)
{
    QSettings settings;

    settings.setValue(scope + "/preprocess/hasDownscale",
                      downscale_config.hasDownscale);

    settings.setValue(scope + "/preprocess/downscaleFactor",
                      downscale_config.downscaleFactor);
}

void ApplicationSettings::set_img_display_config(const DisplayConfig& disp_config)
{
    imgConfig.display = disp_config;
    emit imgDisplaySettingsChanged(imgConfig.display);
}

void ApplicationSettings::set_cam_display_config(const DisplayConfig& disp_config)
{
    camConfig.display = disp_config;
    emit camDisplaySettingsChanged(camConfig.display);
}

void ApplicationSettings::save_disp(const QString& scope,
                                    const DisplayConfig& disp_config)
{
    QSettings settings;


    settings.setValue(scope + "/display/l_out_displayed",
                      disp_config.l_out_displayed);

    settings.setValue(scope + "/display/l_out_red",
                      disp_config.l_out_color.red);
    settings.setValue(scope + "/display/l_out_green",
                      disp_config.l_out_color.green);
    settings.setValue(scope + "/display/l_out_blue",
                      disp_config.l_out_color.blue);

    settings.setValue(scope + "/display/l_in_displayed",
                      disp_config.l_in_displayed);

    settings.setValue(scope + "/display/l_in_red",
                      disp_config.l_in_color.red);
    settings.setValue(scope + "/display/l_in_green",
                      disp_config.l_in_color.green);
    settings.setValue(scope + "/display/l_in_blue",
                      disp_config.l_in_color.blue);

    settings.setValue(scope + "/display/algorithm_overlay",
                      disp_config.algorithm_overlay);

    settings.setValue(scope + "/display/input_displayed",
                      disp_config.input_displayed);

    settings.setValue(scope + "/display/flip_horizontal",
                      disp_config.flip_horizontal);
}

void ApplicationSettings::load_img_session_config()
{
    QSettings settings;

    load_algo("img",
              imgConfig.compute.algo);

    // force StopOnFailure for image session,
    // this parameter is not persistant
    imgConfig.compute.algo.acConfig.failure_mode = ofeli_ip::FailureHandlingMode::StopOnFailure;

    bool isOk = load_initial_phi();

    if ( !isOk )
    {
        load_default_initial_phi();
    }

    // preprocess
    load_downscale("img",
                   imgConfig.compute.downscale);

    auto& fc = imgConfig.compute.processing;

    fc.has_gaussian_noise = settings.value("img/preprocess/has_gaussian_noise", false).toBool();
    fc.std_noise = settings.value("img/preprocess/std_noise", 20.f).toFloat();
    fc.has_salt_noise = settings.value("img/preprocess/has_salt_noise", false).toBool();
    fc.proba_noise = settings.value("img/preprocess/proba_noise", 0.05f).toFloat();
    fc.has_speckle_noise = settings.value("img/preprocess/has_speckle_noise", false).toBool();
    fc.std_speckle_noise = settings.value("img/preprocess/std_speckle_noise", 0.16f).toFloat();

    fc.has_median_filt = settings.value("img/preprocess/has_median_filt", false).toBool();
    fc.kernel_median_length = settings.value("img/preprocess/kernel_median_length", 5).toInt();
    fc.has_O1_algo = settings.value("img/preprocess/has_O1_algo", true).toBool();
    fc.has_mean_filt = settings.value("img/preprocess/has_mean_filt", false).toBool();
    fc.kernel_mean_length = settings.value("img/preprocess/kernel_mean_length", 5).toInt();
    fc.has_gaussian_filt = settings.value("img/preprocess/has_gaussian_filt", false).toBool();
    fc.kernel_gaussian_length = settings.value("img/preprocess/kernel_gaussian_length", 5).toInt();
    fc.sigma = settings.value("img/preprocess/sigma", 2.f).toFloat();

    fc.has_aniso_diff = settings.value("img/preprocess/has_aniso_diff", false).toBool();
    fc.aniso_option = ofeli_ip::AnisoDiff( settings.value("img/preprocess/aniso_option", ofeli_ip::AnisoDiff::FUNCTION1).toUInt() );
    fc.max_itera = settings.value("img/preprocess/max_itera", 10).toInt();
    fc.lambda = settings.value("img/preprocess/lambda", 1.f/7.f).toFloat();
    fc.kappa = settings.value("img/preprocess/kappa", 30.f).toFloat();

    fc.has_open_filt = settings.value("img/preprocess/has_open_filt", false).toBool();
    fc.kernel_open_length = settings.value("img/preprocess/kernel_open_length", 5).toInt();
    fc.has_close_filt = settings.value("img/preprocess/has_close_filt", false).toBool();
    fc.kernel_close_length = settings.value("img/preprocess/kernel_close_length", 5).toInt();
    fc.has_top_hat_filt = settings.value("img/preprocess/has_top_hat_filt", false).toBool();
    fc.is_white_top_hat = settings.value("img/preprocess/is_white_top_hat", true).toBool();
    fc.kernel_tophat_length = settings.value("img/preprocess/kernel_tophat_length", 5).toInt();

    fc.has_O1_morpho = settings.value("img/preprocess/has_O1_morpho", true).toBool();

    // display
    load_disp("img",
              imgConfig.display);
}

void ApplicationSettings::load_cam_session_config()
{
    QSettings settings;

    load_algo("cam",
              camConfig.compute.algo);

    // force RecoverOnFailure for camera session
    // this parameter is not persistant
    camConfig.compute.algo.acConfig.failure_mode = ofeli_ip::FailureHandlingMode::RecoverOnFailure;

    camConfig.compute.cyclesNbr = settings.value("cam/algo/cyclesNbr", 3).toInt();

    // preprocess
    load_downscale("cam",
                   camConfig.compute.downscale);

    camConfig.compute.hasTemporalFiltering = settings.value("cam/preprocess/hasTemporalFiltering",
                                                            true).toBool();

    // display
    load_disp("cam",
              camConfig.display);
}

void ApplicationSettings::load_algo(const QString& scope,
                                    AlgoConfig& algo_config)
{
    QSettings settings;

    algo_config.connectivity = static_cast<ofeli_ip::Connectivity>(settings.value(scope + "/algo/connectivity",
                                                                      int(ofeli_ip::Connectivity::Four)).toInt());
    algo_config.acConfig.Na = settings.value(scope + "/algo/Na", 30).toInt();
    algo_config.acConfig.is_cycle2 = settings.value(scope + "/algo/has_smoothing_cycle", true).toBool();
    algo_config.acConfig.Ns = settings.value(scope + "/algo/Ns", 3).toInt();
    algo_config.acConfig.disk_radius = settings.value(scope + "/algo/disk_radius", 2).toInt();


    algo_config.regionAcConfig.lambda_out = settings.value(scope + "/algo/lambda_out", 1).toInt();
    algo_config.regionAcConfig.lambda_in = settings.value(scope + "/algo/lambda_in", 1).toInt();

    algo_config.regionAcConfig.color_space = ofeli_ip::ColorSpaceOption(settings.value(
                scope + "/algo/color_space", int(ofeli_ip::ColorSpaceOption::RGB)).toInt());

    algo_config.regionAcConfig.weights.c1 = settings.value(scope + "/algo/alpha", 1).toInt();
    algo_config.regionAcConfig.weights.c2 = settings.value(scope + "/algo/beta",  1).toInt();
    algo_config.regionAcConfig.weights.c3 = settings.value(scope + "/algo/gamma", 1).toInt();
}

void ApplicationSettings::load_downscale(const QString& scope,
                                       DownscaleConfig& downscale_config)
{
    QSettings settings;

    bool default_is_downscale = false;

    if (scope.toLower() == "cam")
    {
        default_is_downscale = true;
    }

    downscale_config.hasDownscale = settings.value(scope + "/preprocess/hasDownscale", default_is_downscale).toBool();
    downscale_config.downscaleFactor = settings.value(scope + "/preprocess/downscaleFactor", 2).toInt();
}
void ApplicationSettings::load_disp(const QString& scope,
                                    DisplayConfig& disp_config)
{
    QSettings settings;

    disp_config.l_out_displayed = settings.value(scope + "/display/l_out_displayed", true).toBool();
    disp_config.l_out_color.red = settings.value(scope + "/display/l_out_red", 0u).toUInt();
    disp_config.l_out_color.green = settings.value(scope + "/display/l_out_green", 0u).toUInt();
    disp_config.l_out_color.blue = settings.value(scope + "/display/l_out_blue", 255u).toUInt();

    disp_config.l_in_displayed = settings.value(scope + "/display/l_in_displayed", true).toBool();
    disp_config.l_in_color.red = settings.value(scope + "/display/l_in_red", 255u).toUInt();
    disp_config.l_in_color.green = settings.value(scope + "/display/l_in_green", 0u).toUInt();
    disp_config.l_in_color.blue = settings.value(scope + "/display/l_in_blue", 0u).toUInt();

    disp_config.algorithm_overlay = settings.value(scope + "/display/algorithm_overlay", true).toBool();
    disp_config.input_displayed   = settings.value(scope + "/display/input_displayed", false).toBool();
    disp_config.flip_horizontal   = settings.value(scope + "/display/flip_horizontal", false).toBool();
}

QDir ApplicationSettings::settingsDirectory()
{
    QSettings settings;
    QFileInfo info(settings.fileName());
    return info.dir();
}

bool ApplicationSettings::load_initial_phi()
{
    bool isOk = false;

    QDir dir = settingsDirectory();
    QString phiPath = dir.filePath(kPhiInitFilename);

    QImage& img = imgConfig.compute.initialPhi;

    if (QFile::exists(phiPath))
    {
        img.load(phiPath);
    }

    if ( !img.isNull() &&
         img.format() !=  QImage::Format_Grayscale8 )
    {
        img = img.convertToFormat(QImage::Format_Grayscale8);
    }

    if ( !img.isNull() &&
         img.format() == QImage::Format_Grayscale8 )
    {
        isOk = true;
    }

    return isOk;
}

void ApplicationSettings::load_default_initial_phi()
{
    const int width  = 1280;
    const int height = 720;

    imgConfig.compute.initialPhi = QImage(width, height, QImage::Format_Grayscale8);
    imgConfig.compute.initialPhi.fill(Qt::black);

    QPainter painter(&imgConfig.compute.initialPhi);

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);

    const int ellipseWidth  = static_cast<int>(0.85 * width);
    const int ellipseHeight = static_cast<int>(0.85 * height);

    QRect boundingBox(
        width  / 2 - ellipseWidth  / 2,
        height / 2 - ellipseHeight / 2,
        ellipseWidth,
        ellipseHeight
        );

    painter.drawEllipse(boundingBox);
}

bool ApplicationSettings::save_initial_phi()
{
    bool isOk = false;

    QImage img = imgConfig.compute.initialPhi;

    if ( !img.isNull() &&
         img.format() == QImage::Format_Grayscale8 )
    {
        QDir dir = settingsDirectory();
        QString phiPath = dir.filePath(kPhiInitFilename);


        isOk = img.save(phiPath, "PNG");
    }

    return isOk;
}

void ApplicationSettings::resize_initial_phi(int width, int height)
{
    QImage& img = imgConfig.compute.initialPhi;

    if ( !img.isNull() )
    {
        if ( width != img.width() || height != img.height() )
        {
            img = img.scaled( width, height,
                              Qt::IgnoreAspectRatio,
                              Qt::FastTransformation );

            emit resizedPhi( img );
        }
    }
}

}
