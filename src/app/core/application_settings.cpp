// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "application_settings.hpp"
#include "ac_types.hpp"

#include <QPainter>
#include <QSettings>
#include <QStandardPaths>

namespace fluvel_app
{

namespace
{
static constexpr const char* kPhiInitFilename = "initial_phi.png";

QSettings userSettings()
{
    return QSettings();
}

QSettings imgSessionSettings()
{
    QSettings base;
    QFileInfo info(base.fileName());
    QString dir = info.dir().absolutePath();

    return QSettings(dir + "/image_session.ini", QSettings::IniFormat);
}

QSettings camSessionSettings()
{
    QSettings base;
    QFileInfo info(base.fileName());
    QString dir = info.dir().absolutePath();

    return QSettings(dir + "/camera_session.ini", QSettings::IniFormat);
}

QSettings sessionSettings(fluvel_app::Session session)
{
    switch (session)
    {
        case fluvel_app::Session::Image:
            return imgSessionSettings();
        case fluvel_app::Session::Camera:
            return camSessionSettings();
    }
    Q_UNREACHABLE();
}

QString rgbToString(const fluvel_ip::Rgb_uc& c)
{
    return QString("%1,%2,%3").arg(c.red).arg(c.green).arg(c.blue);
}

fluvel_ip::Rgb_uc rgbFromString(const QString& s, const fluvel_ip::Rgb_uc& defaultColor)
{
    auto clampToByte = [](int v) -> uint8_t
    {
        if (v < 0)
            return 0;
        if (v > 255)
            return 255;
        return static_cast<uint8_t>(v);
    };

    QStringList rgb = s.split(',');

    if (rgb.size() != 3)
        return defaultColor;

    fluvel_ip::Rgb_uc c;
    c.red = clampToByte(rgb[0].toInt());
    c.green = clampToByte(rgb[1].toInt());
    c.blue = clampToByte(rgb[2].toInt());

    return c;
}

} // namespace

ApplicationSettings::ApplicationSettings()
{
    QSettings settings = userSettings();

    const QString s = settings.value("ui/language", "system").toString();

    app_language = language_from_string(s.toStdString());

    load_img_session_config();
    load_cam_session_config();
}

void ApplicationSettings::save()
{
    QSettings settings = userSettings();

    settings.setValue("ui/language", to_string(app_language));

    save_img_session_config();
    save_cam_session_config();
}

void ApplicationSettings::save_img_session_config()
{
    QSettings settings = imgSessionSettings();

    // algo

    save_algo(Session::Image, imgConfig.compute.algo);

    // phi init used only for image session
    save_initial_phi();

    save_downscale(Session::Image, imgConfig.compute.downscale);

    settings.setValue("preprocess/enabled", imgConfig.compute.processing.enabled);

    settings.setValue("preprocess/has_gaussian_noise",
                      imgConfig.compute.processing.has_gaussian_noise);
    settings.setValue("preprocess/std_noise", imgConfig.compute.processing.std_noise);
    settings.setValue("preprocess/has_salt_noise", imgConfig.compute.processing.has_salt_noise);
    settings.setValue("preprocess/proba_noise", imgConfig.compute.processing.proba_noise);
    settings.setValue("preprocess/has_speckle_noise",
                      imgConfig.compute.processing.has_speckle_noise);
    settings.setValue("preprocess/std_speckle_noise",
                      imgConfig.compute.processing.std_speckle_noise);

    settings.setValue("preprocess/has_median_filt", imgConfig.compute.processing.has_median_filt);
    settings.setValue("preprocess/kernel_median_length",
                      imgConfig.compute.processing.kernel_median_length);

    settings.setValue("preprocess/has_O1_algo", imgConfig.compute.processing.has_O1_algo);
    settings.setValue("preprocess/has_mean_filt", imgConfig.compute.processing.has_mean_filt);
    settings.setValue("preprocess/kernel_mean_length",
                      imgConfig.compute.processing.kernel_mean_length);
    settings.setValue("preprocess/has_gaussian_filt",
                      imgConfig.compute.processing.has_gaussian_filt);
    settings.setValue("preprocess/kernel_gaussian_length",
                      imgConfig.compute.processing.kernel_gaussian_length);
    settings.setValue("preprocess/sigma", imgConfig.compute.processing.sigma);

    settings.setValue("preprocess/has_aniso_diff", imgConfig.compute.processing.has_aniso_diff);
    settings.setValue("preprocess/aniso_option", imgConfig.compute.processing.aniso_option);
    settings.setValue("preprocess/max_itera", imgConfig.compute.processing.max_itera);
    settings.setValue("preprocess/lambda", imgConfig.compute.processing.lambda);
    settings.setValue("preprocess/kappa", imgConfig.compute.processing.kappa);

    settings.setValue("preprocess/has_open_filt", imgConfig.compute.processing.has_open_filt);
    settings.setValue("preprocess/kernel_open_length",
                      imgConfig.compute.processing.kernel_open_length);

    settings.setValue("preprocess/has_close_filt", imgConfig.compute.processing.has_close_filt);
    settings.setValue("preprocess/kernel_close_length",
                      imgConfig.compute.processing.kernel_close_length);

    settings.setValue("preprocess/has_top_hat_filt", imgConfig.compute.processing.has_top_hat_filt);
    settings.setValue("preprocess/is_white_top_hat", imgConfig.compute.processing.is_white_top_hat);
    settings.setValue("preprocess/kernel_tophat_length",
                      imgConfig.compute.processing.kernel_tophat_length);

    settings.setValue("preprocess/has_O1_morpho", imgConfig.compute.processing.has_O1_morpho);

    // display

    save_disp(Session::Image, imgConfig.display);

    emit imgSettingsChanged(imgConfig);
}

void ApplicationSettings::save_img_session_config_with_val(const ImageSessionSettings& config)
{
    imgConfig = config;

    save_img_session_config();
}

void ApplicationSettings::save_cam_session_config()
{
    QSettings settings = camSessionSettings();

    // algo

    save_algo(Session::Camera, camConfig.compute.algo);

    settings.setValue("algo/cycles_nbr", camConfig.compute.cyclesNbr);

    // preprocess

    save_downscale(Session::Camera, camConfig.compute.downscale);

    settings.setValue("preprocess/has_temporal_filtering", camConfig.compute.hasTemporalFiltering);

    // display

    save_disp(Session::Camera, camConfig.display);

    emit videoSettingsChanged(camConfig);
}

void ApplicationSettings::save_cam_session_config_with_val(const VideoSessionSettings& config)
{
    camConfig = config;

    save_cam_session_config();
}

void ApplicationSettings::save_algo(Session session, const AlgoConfig& algo_config)
{
    QSettings settings = sessionSettings(session);

    settings.setValue("algo/connectivity", fluvel_ip::to_string(algo_config.connectivity));

    settings.setValue("algo/Na", algo_config.acConfig.Na);

    settings.setValue("algo/has_smoothing_cycle", algo_config.acConfig.hasCycle2);

    settings.setValue("algo/Ns", algo_config.acConfig.Ns);

    settings.setValue("algo/disk_radius", algo_config.acConfig.diskRadius);

    settings.setValue("algo/lambda_out", algo_config.regionAcConfig.lambdaOut);

    settings.setValue("algo/lambda_in", algo_config.regionAcConfig.lambdaIn);

    settings.setValue("algo/color_space",
                      fluvel_ip::to_string(algo_config.regionAcConfig.color_space));

    settings.setValue("algo/color_weight_w1", algo_config.regionAcConfig.weights.c1);

    settings.setValue("algo/color_weight_w2", algo_config.regionAcConfig.weights.c2);

    settings.setValue("algo/color_weight_w3", algo_config.regionAcConfig.weights.c3);
}

void ApplicationSettings::save_downscale(Session session, const DownscaleConfig& downscale_config)
{
    QSettings settings = sessionSettings(session);

    settings.setValue("preprocess/has_downscale", downscale_config.hasDownscale);

    settings.setValue("preprocess/downscale_factor", downscale_config.downscaleFactor);
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

void ApplicationSettings::save_disp(Session session, const DisplayConfig& disp_config)
{
    QSettings settings = sessionSettings(session);

    settings.setValue("display/contour_out/enabled", disp_config.l_out_displayed);
    settings.setValue("display/contour_out/rgb", rgbToString(disp_config.l_out_color));

    settings.setValue("display/contour_in/enabled", disp_config.l_in_displayed);
    settings.setValue("display/contour_in/rgb", rgbToString(disp_config.l_in_color));

    settings.setValue("display/algorithm_overlay", disp_config.algorithm_overlay);

    settings.setValue("display/base_image", to_string(disp_config.image));

    settings.setValue("display/mirror_mode", disp_config.mirrorMode);

    settings.setValue("display/smooth_display", disp_config.smoothDisplay);
}

void ApplicationSettings::load_img_session_config()
{
    QSettings settings = imgSessionSettings();

    load_algo(Session::Image, imgConfig.compute.algo);

    bool isOk = load_initial_phi();

    if (!isOk)
    {
        load_default_initial_phi();
    }

    // preprocess
    load_downscale(Session::Image, imgConfig.compute.downscale);

    auto& fc = imgConfig.compute.processing;

    fc.enabled = settings.value("preprocess/enabled", false).toBool();

    fc.has_gaussian_noise =
        settings.value("preprocess/has_gaussian_noise", ProcessingConfig::kDefaultProcess).toBool();
    fc.std_noise =
        settings.value("preprocess/std_noise", ProcessingConfig::kDefaultStdNoise).toFloat();
    fc.has_salt_noise =
        settings.value("preprocess/has_salt_noise", ProcessingConfig::kDefaultProcess).toBool();
    fc.proba_noise =
        settings.value("preprocess/proba_noise", ProcessingConfig::kDefaultSaltNoise).toFloat();
    fc.has_speckle_noise =
        settings.value("preprocess/has_speckle_noise", ProcessingConfig::kDefaultProcess).toBool();
    fc.std_speckle_noise =
        settings.value("preprocess/std_speckle_noise", ProcessingConfig::kDefaultSpeckleNoise)
            .toFloat();

    fc.has_median_filt =
        settings.value("preprocess/has_median_filt", ProcessingConfig::kDefaultProcess).toBool();
    fc.kernel_median_length =
        settings.value("preprocess/kernel_median_length", ProcessingConfig::kDefaultKernelLength)
            .toInt();
    fc.has_O1_algo =
        settings.value("preprocess/has_O1_algo", ProcessingConfig::kDefault01Algo).toBool();
    fc.has_mean_filt =
        settings.value("preprocess/has_mean_filt", ProcessingConfig::kDefaultProcess).toBool();
    fc.kernel_mean_length =
        settings.value("preprocess/kernel_mean_length", ProcessingConfig::kDefaultKernelLength)
            .toInt();
    fc.has_gaussian_filt =
        settings.value("preprocess/has_gaussian_filt", ProcessingConfig::kDefaultProcess).toBool();
    fc.kernel_gaussian_length =
        settings.value("preprocess/kernel_gaussian_length", ProcessingConfig::kDefaultKernelLength)
            .toInt();
    fc.sigma =
        settings.value("preprocess/sigma", ProcessingConfig::kDefaultGaussianSigma).toFloat();

    fc.has_aniso_diff =
        settings.value("preprocess/has_aniso_diff", ProcessingConfig::kDefaultProcess).toBool();
    fc.aniso_option = fluvel_ip::AnisoDiff(
        settings.value("preprocess/aniso_option", ProcessingConfig::kDefaultAnisoOption).toInt());
    fc.max_itera =
        settings.value("preprocess/max_itera", ProcessingConfig::kDefaultMaxItera).toInt();
    fc.lambda = settings.value("preprocess/lambda", ProcessingConfig::kDefaultLambda).toFloat();
    fc.kappa = settings.value("preprocess/kappa", ProcessingConfig::kDefaultKappa).toFloat();

    fc.has_open_filt =
        settings.value("preprocess/has_open_filt", ProcessingConfig::kDefaultProcess).toBool();
    fc.kernel_open_length =
        settings.value("preprocess/kernel_open_length", ProcessingConfig::kDefaultKernelLength)
            .toInt();
    fc.has_close_filt =
        settings.value("preprocess/has_close_filt", ProcessingConfig::kDefaultProcess).toBool();
    fc.kernel_close_length =
        settings.value("preprocess/kernel_close_length", ProcessingConfig::kDefaultKernelLength)
            .toInt();
    fc.has_top_hat_filt =
        settings.value("preprocess/has_top_hat_filt", ProcessingConfig::kDefaultProcess).toBool();
    fc.is_white_top_hat =
        settings.value("preprocess/is_white_top_hat", ProcessingConfig::kDefaultWhiteTopHat)
            .toBool();
    fc.kernel_tophat_length =
        settings.value("preprocess/kernel_tophat_length", ProcessingConfig::kDefaultKernelLength)
            .toInt();

    fc.has_O1_morpho =
        settings.value("preprocess/has_O1_morpho", ProcessingConfig::kDefault01Algo).toBool();

    // display
    load_disp(Session::Image, imgConfig.display);
}

void ApplicationSettings::load_cam_session_config()
{
    QSettings settings = camSessionSettings();

    load_algo(Session::Camera, camConfig.compute.algo);

    camConfig.compute.cyclesNbr =
        settings.value("algo/cycles_nbr", VideoComputeConfig::kDefaultCyclesNbr).toInt();

    // preprocess
    load_downscale(Session::Camera, camConfig.compute.downscale);

    camConfig.compute.hasTemporalFiltering =
        settings
            .value("preprocess/has_temporal_filtering",
                   VideoComputeConfig::kDefaultHasTemporalFiltering)
            .toBool();

    // display
    load_disp(Session::Camera, camConfig.display);
}

void ApplicationSettings::load_algo(Session session, AlgoConfig& algo_config)
{
    QSettings settings = sessionSettings(session);

    const QString s =
        settings.value("algo/connectivity", fluvel_ip::to_string(AlgoConfig::kDefaultConnectivity))
            .toString();

    algo_config.connectivity = fluvel_ip::connectivity_from_string(s.toStdString());

    algo_config.acConfig.Na = settings.value("algo/Na", fluvel_ip::AcConfig::kDefaultNa).toInt();
    algo_config.acConfig.hasCycle2 =
        settings.value("algo/has_smoothing_cycle", fluvel_ip::AcConfig::kDefaultIsCycle2).toBool();
    algo_config.acConfig.Ns = settings.value("algo/Ns", fluvel_ip::AcConfig::kDefaultNs).toInt();
    algo_config.acConfig.diskRadius =
        settings.value("algo/disk_radius", fluvel_ip::AcConfig::kDefaultDiskRadius).toInt();

    switch (session)
    {
        case Session::Camera:
            algo_config.acConfig.failureMode = fluvel_ip::FailureHandlingMode::RecoverOnFailure;
            break;

        case Session::Image:
        default:
            algo_config.acConfig.failureMode = fluvel_ip::FailureHandlingMode::StopOnFailure;
            break;
    }

    algo_config.regionAcConfig.lambdaOut =
        settings.value("algo/lambda_out", fluvel_ip::RegionConfig::kDefaultLambdaOut).toInt();
    algo_config.regionAcConfig.lambdaIn =
        settings.value("algo/lambda_in", fluvel_ip::RegionConfig::kDefaultLambdaIn).toInt();

    const QString s_cs =
        settings
            .value("algo/color_space",
                   fluvel_ip::to_string(fluvel_ip::RegionColorConfig::kDefaultColorSpace))
            .toString();

    algo_config.regionAcConfig.color_space = fluvel_ip::color_space_from_string(s_cs.toStdString());

    algo_config.regionAcConfig.weights.c1 =
        settings.value("algo/color_weight_w1", fluvel_ip::RegionColorConfig::kDefaultWeights.c1)
            .toInt();
    algo_config.regionAcConfig.weights.c2 =
        settings.value("algo/color_weight_w2", fluvel_ip::RegionColorConfig::kDefaultWeights.c2)
            .toInt();
    algo_config.regionAcConfig.weights.c3 =
        settings.value("algo/color_weight_w3", fluvel_ip::RegionColorConfig::kDefaultWeights.c3)
            .toInt();
}

void ApplicationSettings::load_downscale(Session session, DownscaleConfig& downscale_config)
{
    QSettings settings = sessionSettings(session);

    bool defaultIsDownscale = (session == Session::Camera);

    downscale_config.hasDownscale =
        settings.value("preprocess/has_downscale", defaultIsDownscale).toBool();

    downscale_config.downscaleFactor =
        settings.value("preprocess/downscale_factor", DownscaleConfig::kDefaultDownscaleFactor)
            .toInt();
}
void ApplicationSettings::load_disp(Session session, DisplayConfig& disp_config)
{
    auto clampToByte = [](int v) -> uint8_t
    {
        if (v < 0)
            return 0;
        if (v > 255)
            return 255;
        return static_cast<uint8_t>(v);
    };

    QSettings settings = sessionSettings(session);

    disp_config.l_out_displayed =
        settings.value("display/contour_out/enabled", DisplayConfig::kDefaultListDisplayed)
            .toBool();
    disp_config.l_out_color = rgbFromString(
        settings.value("display/contour_out/rgb", rgbToString(DisplayConfig::kDefaultOut))
            .toString(),
        DisplayConfig::kDefaultOut);

    disp_config.l_in_displayed =
        settings.value("display/contour_in/enabled", DisplayConfig::kDefaultListDisplayed).toBool();
    disp_config.l_in_color = rgbFromString(
        settings.value("display/contour_in/rgb", rgbToString(DisplayConfig::kDefaultIn)).toString(),
        DisplayConfig::kDefaultIn);

    disp_config.algorithm_overlay =
        settings.value("display/algorithm_overlay", DisplayConfig::kDefaultOverlay).toBool();

    const QString s =
        settings
            .value("display/base_image",
                   QString::fromStdString(to_string(DisplayConfig::kDefaultImageBase)))
            .toString();

    disp_config.image = ib_from_string(s.toStdString());

    disp_config.mirrorMode =
        settings.value("display/mirror_mode", DisplayConfig::kDefaultOptions).toBool();

    disp_config.smoothDisplay =
        settings.value("display/smooth_display", DisplayConfig::kDefaultOptions).toBool();
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

    if (!img.isNull() && img.format() != QImage::Format_Grayscale8)
    {
        img = img.convertToFormat(QImage::Format_Grayscale8);
    }

    if (!img.isNull() && img.format() == QImage::Format_Grayscale8)
    {
        isOk = true;
    }

    return isOk;
}

void ApplicationSettings::load_default_initial_phi()
{
    const int width = 1280;
    const int height = 720;

    imgConfig.compute.initialPhi = QImage(width, height, QImage::Format_Grayscale8);
    imgConfig.compute.initialPhi.fill(Qt::black);

    QPainter painter(&imgConfig.compute.initialPhi);

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);

    const int ellipseWidth = static_cast<int>(0.85 * width);
    const int ellipseHeight = static_cast<int>(0.85 * height);

    QRect boundingBox(width / 2 - ellipseWidth / 2, height / 2 - ellipseHeight / 2, ellipseWidth,
                      ellipseHeight);

    painter.drawEllipse(boundingBox);
}

bool ApplicationSettings::save_initial_phi()
{
    bool isOk = false;

    QImage img = imgConfig.compute.initialPhi;

    if (!img.isNull() && img.format() == QImage::Format_Grayscale8)
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

    if (!img.isNull())
    {
        if (width != img.width() || height != img.height())
        {
            img = img.scaled(width, height, Qt::IgnoreAspectRatio, Qt::FastTransformation);

            emit resizedPhi(img);
        }
    }
}

QString toSettingsPrefix(Session scope)
{
    switch (scope)
    {
        case Session::Camera:
            return "cam";
        case Session::Image:
            return "img";
    }
    Q_UNREACHABLE();
}

} // namespace fluvel_app
