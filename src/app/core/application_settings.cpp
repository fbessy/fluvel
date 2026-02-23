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

#include <utility>

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

    save_algo(Session::Image,
              imgConfig.compute.algo);

    // phi init used only for image session
    save_initial_phi();

    save_downscale(Session::Image,
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

    save_disp(Session::Image,
              imgConfig.display);

    emit imgSettingsChanged(imgConfig);
}

void ApplicationSettings::save_cam_session_config()
{
    QSettings settings;

    // algo

    save_algo(Session::Camera,
              camConfig.compute.algo);

    settings.setValue("cam/algo/cyclesNbr",
                      camConfig.compute.cyclesNbr);

    // preprocess

    save_downscale(Session::Camera,
                   camConfig.compute.downscale);

    settings.setValue("cam/preprocess/hasTemporalFiltering",
                      camConfig.compute.hasTemporalFiltering);

    // display

    save_disp(Session::Camera,
              camConfig.display);

    emit videoSettingsChanged(camConfig);
}

void ApplicationSettings::save_algo(Session session,
                                    const AlgoConfig& algo_config)
{
    QSettings settings;

    const QString scope = toSettingsPrefix(session);

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

void ApplicationSettings::save_downscale(Session session,
                                         const DownscaleConfig& downscale_config)
{
    QSettings settings;

    const QString scope = toSettingsPrefix(session);

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
    emit videoDisplaySettingsChanged(camConfig.display);
}

void ApplicationSettings::save_disp(Session session,
                                    const DisplayConfig& disp_config)
{
    QSettings settings;

    const QString scope = toSettingsPrefix(session);

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

    settings.setValue(scope + "/display/image",
                      static_cast<int>(disp_config.image));

    settings.setValue(scope + "/display/mirrorMode",
                      disp_config.mirrorMode);

    settings.setValue(scope + "/display/smoothDisplay",
                      disp_config.smoothDisplay);
}

void ApplicationSettings::load_img_session_config()
{
    QSettings settings;

    load_algo(Session::Image,
              imgConfig.compute.algo);

    bool isOk = load_initial_phi();

    if ( !isOk )
    {
        load_default_initial_phi();
    }

    // preprocess
    load_downscale(Session::Image,
                   imgConfig.compute.downscale);

    auto& fc = imgConfig.compute.processing;

    fc.has_gaussian_noise = settings.value("img/preprocess/has_gaussian_noise",
                                           ProcessingConfig::defaultProcess).toBool();
    fc.std_noise = settings.value("img/preprocess/std_noise",
                                  ProcessingConfig::defaultStdNoise).toFloat();
    fc.has_salt_noise = settings.value("img/preprocess/has_salt_noise",
                                       ProcessingConfig::defaultProcess).toBool();
    fc.proba_noise = settings.value("img/preprocess/proba_noise",
                                    ProcessingConfig::defaultSaltNoise).toFloat();
    fc.has_speckle_noise = settings.value("img/preprocess/has_speckle_noise",
                                          ProcessingConfig::defaultProcess).toBool();
    fc.std_speckle_noise = settings.value("img/preprocess/std_speckle_noise",
                                          ProcessingConfig::defaultSpeckleNoise).toFloat();

    fc.has_median_filt = settings.value("img/preprocess/has_median_filt",
                                        ProcessingConfig::defaultProcess).toBool();
    fc.kernel_median_length = settings.value("img/preprocess/kernel_median_length",
                                             ProcessingConfig::defaultKernelLength).toInt();
    fc.has_O1_algo = settings.value("img/preprocess/has_O1_algo",
                                    ProcessingConfig::default01Algo).toBool();
    fc.has_mean_filt = settings.value("img/preprocess/has_mean_filt",
                                      ProcessingConfig::defaultProcess).toBool();
    fc.kernel_mean_length = settings.value("img/preprocess/kernel_mean_length",
                                           ProcessingConfig::defaultKernelLength).toInt();
    fc.has_gaussian_filt = settings.value("img/preprocess/has_gaussian_filt",
                                          ProcessingConfig::defaultProcess).toBool();
    fc.kernel_gaussian_length = settings.value("img/preprocess/kernel_gaussian_length",
                                               ProcessingConfig::defaultKernelLength).toInt();
    fc.sigma = settings.value("img/preprocess/sigma",
                              ProcessingConfig::defaultGaussianSigma).toFloat();

    fc.has_aniso_diff = settings.value("img/preprocess/has_aniso_diff",
                                       ProcessingConfig::defaultProcess).toBool();
    fc.aniso_option = ofeli_ip::AnisoDiff( settings.value("img/preprocess/aniso_option",
                                                         ProcessingConfig::defaultAnisoOption).toUInt() );
    fc.max_itera = settings.value("img/preprocess/max_itera",
                                  ProcessingConfig::defaultMaxItera).toInt();
    fc.lambda = settings.value("img/preprocess/lambda",
                               ProcessingConfig::defaultLambda).toFloat();
    fc.kappa = settings.value("img/preprocess/kappa",
                              ProcessingConfig::defaultKappa).toFloat();

    fc.has_open_filt = settings.value("img/preprocess/has_open_filt",
                                      ProcessingConfig::defaultProcess).toBool();
    fc.kernel_open_length = settings.value("img/preprocess/kernel_open_length",
                                           ProcessingConfig::defaultKernelLength).toInt();
    fc.has_close_filt = settings.value("img/preprocess/has_close_filt",
                                       ProcessingConfig::defaultProcess).toBool();
    fc.kernel_close_length = settings.value("img/preprocess/kernel_close_length",
                                            ProcessingConfig::defaultKernelLength).toInt();
    fc.has_top_hat_filt = settings.value("img/preprocess/has_top_hat_filt",
                                         ProcessingConfig::defaultProcess).toBool();
    fc.is_white_top_hat = settings.value("img/preprocess/is_white_top_hat",
                                         ProcessingConfig::defaultWhiteTopHat).toBool();
    fc.kernel_tophat_length = settings.value("img/preprocess/kernel_tophat_length",
                                             ProcessingConfig::defaultKernelLength).toInt();

    fc.has_O1_morpho = settings.value("img/preprocess/has_O1_morpho",
                                      ProcessingConfig::default01Algo).toBool();

    // display
    load_disp(Session::Image,
              imgConfig.display);
}

void ApplicationSettings::load_cam_session_config()
{
    QSettings settings;

    load_algo(Session::Camera,
              camConfig.compute.algo);

    camConfig.compute.cyclesNbr = settings.value("cam/algo/cyclesNbr",
                                                 VideoComputeConfig::defaultCyclesNbr).toInt();

    // preprocess
    load_downscale(Session::Camera,
                   camConfig.compute.downscale);

    camConfig.compute.hasTemporalFiltering = settings.value("cam/preprocess/hasTemporalFiltering",
                                                            VideoComputeConfig::defaultHasTemporalFiltering).toBool();

    // display
    load_disp(Session::Camera,
              camConfig.display);
}

void ApplicationSettings::load_algo(Session session,
                                    AlgoConfig& algo_config)
{
    QSettings settings;

    const QString scope = toSettingsPrefix(session);

    algo_config.connectivity = static_cast<ofeli_ip::Connectivity>(settings.value(scope + "/algo/connectivity",
                                                                      int(AlgoConfig::defaultConnectivity)).toInt());
    algo_config.acConfig.Na = settings.value(scope + "/algo/Na",
                                             ofeli_ip::AcConfig::default_Na).toInt();
    algo_config.acConfig.is_cycle2 = settings.value(scope + "/algo/has_smoothing_cycle",
                                                    ofeli_ip::AcConfig::default_is_cycle2).toBool();
    algo_config.acConfig.Ns = settings.value(scope + "/algo/Ns",
                                             ofeli_ip::AcConfig::default_Ns).toInt();
    algo_config.acConfig.disk_radius = settings.value(scope + "/algo/disk_radius",
                                                      ofeli_ip::AcConfig::default_disk_radius).toInt();

    switch (session)
    {
    case Session::Camera:
        algo_config.acConfig.failure_mode
            = ofeli_ip::FailureHandlingMode::RecoverOnFailure;
        break;

    case Session::Image:
    default:
        algo_config.acConfig.failure_mode
            = ofeli_ip::FailureHandlingMode::StopOnFailure;
        break;
    }

    algo_config.regionAcConfig.lambda_out = settings.value(scope + "/algo/lambda_out",
                                                           ofeli_ip::RegionConfig::default_lambda_out).toInt();
    algo_config.regionAcConfig.lambda_in = settings.value(scope + "/algo/lambda_in",
                                                          ofeli_ip::RegionConfig::default_lambda_in).toInt();

    algo_config.regionAcConfig.color_space = ofeli_ip::ColorSpaceOption(settings.value(
        scope + "/algo/color_space",
            int(ofeli_ip::RegionColorConfig::default_color_space)).toInt());

    algo_config.regionAcConfig.weights.c1 = settings.value(scope + "/algo/alpha",
                                                           ofeli_ip::RegionColorConfig::default_weights.c1).toInt();
    algo_config.regionAcConfig.weights.c2 = settings.value(scope + "/algo/beta",
                                                           ofeli_ip::RegionColorConfig::default_weights.c2).toInt();
    algo_config.regionAcConfig.weights.c3 = settings.value(scope + "/algo/gamma",
                                                           ofeli_ip::RegionColorConfig::default_weights.c3).toInt();
}

void ApplicationSettings::load_downscale(Session session,
                                         DownscaleConfig& downscale_config)
{
    QSettings settings;

    bool defaultIsDownscale =
        (session == Session::Camera);

    const QString scope = toSettingsPrefix(session);

    downscale_config.hasDownscale = settings.value(scope + "/preprocess/hasDownscale",
                                                   defaultIsDownscale).toBool();

    downscale_config.downscaleFactor = settings.value(scope + "/preprocess/downscaleFactor",
                                                      DownscaleConfig::defaultDownscaleFactor).toInt();
}
void ApplicationSettings::load_disp(Session session,
                                    DisplayConfig& disp_config)
{
    auto clampToByte = [](int v) -> uint8_t
    {
        if (v < 0) return 0;
        if (v > 255) return 255;
        return static_cast<uint8_t>(v);
    };

    QSettings settings;

    const QString scope = toSettingsPrefix(session);

    disp_config.l_out_displayed   = settings.value(scope + "/display/l_out_displayed",
                                                 DisplayConfig::defaultListDisplayed).toBool();
    disp_config.l_out_color.red   = clampToByte( settings.value(scope + "/display/l_out_red",
                                                             DisplayConfig::defaultRedOut).toInt() );
    disp_config.l_out_color.green = clampToByte( settings.value(scope + "/display/l_out_green",
                                                               DisplayConfig::defaultGreenOut).toInt() );
    disp_config.l_out_color.blue  = clampToByte( settings.value(scope + "/display/l_out_blue",
                                                              DisplayConfig::defaultBlueOut).toInt() );

    disp_config.l_in_displayed    = settings.value(scope + "/display/l_in_displayed",
                                                DisplayConfig::defaultListDisplayed).toBool();
    disp_config.l_in_color.red    = clampToByte( settings.value(scope + "/display/l_in_red",
                                                            DisplayConfig::defaultRedIn).toInt() );
    disp_config.l_in_color.green  = clampToByte( settings.value(scope + "/display/l_in_green",
                                                              DisplayConfig::defaultGreenIn).toInt() );
    disp_config.l_in_color.blue   = clampToByte( settings.value(scope + "/display/l_in_blue",
                                                             DisplayConfig::defaultBlueIn).toInt() );

    disp_config.algorithm_overlay = settings.value(scope + "/display/algorithm_overlay",
                                                   DisplayConfig::defaultOverlay).toBool();
    disp_config.image   = static_cast<ImageBase>(settings.value(scope + "/display/image",
                                                 static_cast<int>(DisplayConfig::defaultImage)).toInt());
    disp_config.mirrorMode        = settings.value(scope + "/display/mirrorMode",
                                            DisplayConfig::defaultOptions).toBool();

    disp_config.smoothDisplay     = settings.value(scope + "/display/smoothDisplay",
                                            DisplayConfig::defaultOptions).toBool();
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
                              Qt::SmoothTransformation );

            emit resizedPhi( img );
        }
    }
}

QString toSettingsPrefix(Session scope)
{
    switch (scope)
    {
    case Session::Camera: return "cam";
    case Session::Image:  return "img";
    }
    Q_UNREACHABLE();
}

}
