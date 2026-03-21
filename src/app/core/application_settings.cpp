// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "application_settings.hpp"
#include "ac_types.hpp"

#include <QPainter>
#include <QSettings>
#include <QStandardPaths>

#ifdef FLUVEL_DEBUG
#include "image_debug.hpp"
#endif

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

ApplicationSettings& ApplicationSettings::instance()
{
    static ApplicationSettings instance;
    return instance;
}

ApplicationSettings::ApplicationSettings()
{
    QSettings settings = userSettings();

    const QString s = settings.value("ui/language", "system").toString();

    appLanguage_ = language_from_string(s.toStdString());

    loadImageSessionSettings();
    loadVideoSessionSettings();
}

void ApplicationSettings::save()
{
    QSettings settings = userSettings();

    settings.setValue("ui/language", to_string(appLanguage_));

    saveImageSessionSettings();
    saveVideoSessionSettings();
}

void ApplicationSettings::saveQuiet()
{
    QSignalBlocker blocker(this);
    save();
}

void ApplicationSettings::saveImageSessionSettings()
{
    QSettings settings = imgSessionSettings();

    // algo

    saveAlgo(Session::Image, imageSettings_.compute.algo);

    // phi init used only for image session
    saveInitialPhi();

    saveDownscale(Session::Image, imageSettings_.compute.downscale);

    settings.setValue("preprocess/enabled", imageSettings_.compute.processing.enabled);

    settings.setValue("preprocess/has_gaussian_noise",
                      imageSettings_.compute.processing.has_gaussian_noise);
    settings.setValue("preprocess/std_noise", imageSettings_.compute.processing.std_noise);
    settings.setValue("preprocess/has_salt_noise",
                      imageSettings_.compute.processing.has_salt_noise);
    settings.setValue("preprocess/proba_noise", imageSettings_.compute.processing.proba_noise);
    settings.setValue("preprocess/has_speckle_noise",
                      imageSettings_.compute.processing.has_speckle_noise);
    settings.setValue("preprocess/std_speckle_noise",
                      imageSettings_.compute.processing.std_speckle_noise);

    settings.setValue("preprocess/has_median_filt",
                      imageSettings_.compute.processing.has_median_filt);
    settings.setValue("preprocess/kernel_median_length",
                      imageSettings_.compute.processing.kernel_median_length);

    settings.setValue("preprocess/has_O1_algo", imageSettings_.compute.processing.has_O1_algo);
    settings.setValue("preprocess/has_mean_filt", imageSettings_.compute.processing.has_mean_filt);
    settings.setValue("preprocess/kernel_mean_length",
                      imageSettings_.compute.processing.kernel_mean_length);
    settings.setValue("preprocess/has_gaussian_filt",
                      imageSettings_.compute.processing.has_gaussian_filt);
    settings.setValue("preprocess/kernel_gaussian_length",
                      imageSettings_.compute.processing.kernel_gaussian_length);
    settings.setValue("preprocess/sigma", imageSettings_.compute.processing.sigma);

    settings.setValue("preprocess/has_aniso_diff",
                      imageSettings_.compute.processing.has_aniso_diff);
    settings.setValue("preprocess/aniso_option", imageSettings_.compute.processing.aniso_option);
    settings.setValue("preprocess/max_itera", imageSettings_.compute.processing.max_itera);
    settings.setValue("preprocess/lambda", imageSettings_.compute.processing.lambda);
    settings.setValue("preprocess/kappa", imageSettings_.compute.processing.kappa);

    settings.setValue("preprocess/has_open_filt", imageSettings_.compute.processing.has_open_filt);
    settings.setValue("preprocess/kernel_open_length",
                      imageSettings_.compute.processing.kernel_open_length);

    settings.setValue("preprocess/has_close_filt",
                      imageSettings_.compute.processing.has_close_filt);
    settings.setValue("preprocess/kernel_close_length",
                      imageSettings_.compute.processing.kernel_close_length);

    settings.setValue("preprocess/has_top_hat_filt",
                      imageSettings_.compute.processing.has_top_hat_filt);
    settings.setValue("preprocess/is_white_top_hat",
                      imageSettings_.compute.processing.is_white_top_hat);
    settings.setValue("preprocess/kernel_tophat_length",
                      imageSettings_.compute.processing.kernel_tophat_length);

    settings.setValue("preprocess/has_O1_morpho", imageSettings_.compute.processing.has_O1_morpho);

    // display

    saveDisplay(Session::Image, imageSettings_.display);

    emit imgSettingsChanged(imageSettings_);
}

void ApplicationSettings::setImageSessionSettings(const ImageSessionSettings& config)
{
#ifdef FLUVEL_DEBUG
    qDebug() << __FILE__ << ":" << __LINE__ << __func__
             << "phi:" << image_debug::describeImage(config.compute.initialPhi);
#endif

    imageSettings_ = config;

#ifdef FLUVEL_DEBUG
    qDebug() << __FILE__ << ":" << __LINE__ << __func__
             << "phi:" << image_debug::describeImage(imageSettings_.compute.initialPhi);
#endif

    saveImageSessionSettings();
}

void ApplicationSettings::saveVideoSessionSettings()
{
    QSettings settings = camSessionSettings();

    // input

    settings.setValue("input/use_optimized_format", videoSettings_.compute.useOptimizedFormat);

    saveDownscale(Session::Camera, videoSettings_.compute.downscale);

    settings.setValue("preprocess/has_temporal_filtering",
                      videoSettings_.compute.hasTemporalFiltering);

    // algo

    saveAlgo(Session::Camera, videoSettings_.compute.algo);

    // display

    saveDisplay(Session::Camera, videoSettings_.display);

    emit videoSettingsChanged(videoSettings_);
}

void ApplicationSettings::setVideoSessionSettings(const VideoSessionSettings& config)
{
    videoSettings_ = config;

    saveVideoSessionSettings();
}

void ApplicationSettings::saveAlgo(Session session, const AlgoConfig& algoConfig)
{
    QSettings settings = sessionSettings(session);

    settings.setValue("algo/connectivity", fluvel_ip::to_string(algoConfig.connectivity));

    settings.setValue("algo/Na", algoConfig.acConfig.Na);

    settings.setValue("algo/has_smoothing_cycle", algoConfig.acConfig.hasCycle2);

    settings.setValue("algo/Ns", algoConfig.acConfig.Ns);

    settings.setValue("algo/disk_radius", algoConfig.acConfig.diskRadius);

    settings.setValue("algo/lambda_out", algoConfig.regionAcConfig.lambdaOut);

    settings.setValue("algo/lambda_in", algoConfig.regionAcConfig.lambdaIn);

    settings.setValue("algo/color_space",
                      fluvel_ip::to_string(algoConfig.regionAcConfig.color_space));

    settings.setValue("algo/color_weight_w1", algoConfig.regionAcConfig.weights.c1);

    settings.setValue("algo/color_weight_w2", algoConfig.regionAcConfig.weights.c2);

    settings.setValue("algo/color_weight_w3", algoConfig.regionAcConfig.weights.c3);
}

void ApplicationSettings::saveDownscale(Session session, const DownscaleConfig& downscaleConfig)
{
    QSettings settings = sessionSettings(session);

    settings.setValue("preprocess/has_downscale", downscaleConfig.hasDownscale);

    settings.setValue("preprocess/downscale_factor", downscaleConfig.downscaleFactor);
}

void ApplicationSettings::setImageDisplayConfig(const DisplayConfig& displayConfig)
{
    imageSettings_.display = displayConfig;
    emit imgDisplaySettingsChanged(imageSettings_.display);
}

void ApplicationSettings::setVideoDisplayConfig(const DisplayConfig& displayConfig)
{
    videoSettings_.display = displayConfig;
    emit videoDisplaySettingsChanged(videoSettings_.display);
}

void ApplicationSettings::saveDisplay(Session session, const DisplayConfig& displayConfig)
{
    QSettings settings = sessionSettings(session);

    settings.setValue("display/contour_out/enabled", displayConfig.l_out_displayed);
    settings.setValue("display/contour_out/rgb", rgbToString(displayConfig.l_out_color));

    settings.setValue("display/contour_in/enabled", displayConfig.l_in_displayed);
    settings.setValue("display/contour_in/rgb", rgbToString(displayConfig.l_in_color));

    settings.setValue("display/algorithm_overlay", displayConfig.algorithm_overlay);

    settings.setValue("display/base_image", to_string(displayConfig.image));

    settings.setValue("display/mirror_mode", displayConfig.mirrorMode);

    settings.setValue("display/smooth_display", displayConfig.smoothDisplay);
}

void ApplicationSettings::loadImageSessionSettings()
{
    QSettings settings = imgSessionSettings();

    loadAlgo(Session::Image, imageSettings_.compute.algo);

    bool isOk = loadInitialPhi();

    if (!isOk)
    {
        loadDefaultInitialPhi();
    }

    // preprocess
    loadDownscale(Session::Image, imageSettings_.compute.downscale);

    auto& fc = imageSettings_.compute.processing;

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
    loadDisplay(Session::Image, imageSettings_.display);
}

void ApplicationSettings::loadVideoSessionSettings()
{
    QSettings settings = camSessionSettings();

    // input tab

    videoSettings_.compute.useOptimizedFormat =
        settings.value("input/use_optimized_format", VideoComputeConfig::kDefaultUseOptimizedFormat)
            .toBool();

    loadDownscale(Session::Camera, videoSettings_.compute.downscale);

    videoSettings_.compute.hasTemporalFiltering =
        settings
            .value("preprocess/has_temporal_filtering",
                   VideoComputeConfig::kDefaultHasTemporalFiltering)
            .toBool();

    loadAlgo(Session::Camera, videoSettings_.compute.algo);

    // display
    loadDisplay(Session::Camera, videoSettings_.display);
}

void ApplicationSettings::loadAlgo(Session session, AlgoConfig& algoConfig)
{
    QSettings settings = sessionSettings(session);

    const QString s =
        settings.value("algo/connectivity", fluvel_ip::to_string(AlgoConfig::kDefaultConnectivity))
            .toString();

    algoConfig.connectivity = fluvel_ip::connectivity_from_string(s.toStdString());

    algoConfig.acConfig.Na = settings.value("algo/Na", fluvel_ip::AcConfig::kDefaultNa).toInt();
    algoConfig.acConfig.hasCycle2 =
        settings.value("algo/has_smoothing_cycle", fluvel_ip::AcConfig::kDefaultIsCycle2).toBool();
    algoConfig.acConfig.Ns = settings.value("algo/Ns", fluvel_ip::AcConfig::kDefaultNs).toInt();
    algoConfig.acConfig.diskRadius =
        settings.value("algo/disk_radius", fluvel_ip::AcConfig::kDefaultDiskRadius).toInt();

    switch (session)
    {
        case Session::Camera:
            algoConfig.acConfig.failureMode = fluvel_ip::FailureHandlingMode::RecoverOnFailure;
            break;

        case Session::Image:
        default:
            algoConfig.acConfig.failureMode = fluvel_ip::FailureHandlingMode::StopOnFailure;
            break;
    }

    algoConfig.regionAcConfig.lambdaOut =
        settings.value("algo/lambda_out", fluvel_ip::RegionConfig::kDefaultLambdaOut).toInt();
    algoConfig.regionAcConfig.lambdaIn =
        settings.value("algo/lambda_in", fluvel_ip::RegionConfig::kDefaultLambdaIn).toInt();

    const QString s_cs =
        settings
            .value("algo/color_space",
                   fluvel_ip::to_string(fluvel_ip::RegionColorConfig::kDefaultColorSpace))
            .toString();

    algoConfig.regionAcConfig.color_space = fluvel_ip::color_space_from_string(s_cs.toStdString());

    algoConfig.regionAcConfig.weights.c1 =
        settings.value("algo/color_weight_w1", fluvel_ip::RegionColorConfig::kDefaultWeights.c1)
            .toInt();
    algoConfig.regionAcConfig.weights.c2 =
        settings.value("algo/color_weight_w2", fluvel_ip::RegionColorConfig::kDefaultWeights.c2)
            .toInt();
    algoConfig.regionAcConfig.weights.c3 =
        settings.value("algo/color_weight_w3", fluvel_ip::RegionColorConfig::kDefaultWeights.c3)
            .toInt();
}

void ApplicationSettings::loadDownscale(Session session, DownscaleConfig& downscaleConfig)
{
    QSettings settings = sessionSettings(session);

    bool defaultIsDownscale = (session == Session::Camera);

    downscaleConfig.hasDownscale =
        settings.value("preprocess/has_downscale", defaultIsDownscale).toBool();

    downscaleConfig.downscaleFactor =
        settings.value("preprocess/downscale_factor", DownscaleConfig::kDefaultDownscaleFactor)
            .toInt();
}
void ApplicationSettings::loadDisplay(Session session, DisplayConfig& displayConfig)
{
    QSettings settings = sessionSettings(session);

    displayConfig.l_out_displayed =
        settings.value("display/contour_out/enabled", DisplayConfig::kDefaultListDisplayed)
            .toBool();
    displayConfig.l_out_color = rgbFromString(
        settings.value("display/contour_out/rgb", rgbToString(DisplayConfig::kDefaultOut))
            .toString(),
        DisplayConfig::kDefaultOut);

    displayConfig.l_in_displayed =
        settings.value("display/contour_in/enabled", DisplayConfig::kDefaultListDisplayed).toBool();
    displayConfig.l_in_color = rgbFromString(
        settings.value("display/contour_in/rgb", rgbToString(DisplayConfig::kDefaultIn)).toString(),
        DisplayConfig::kDefaultIn);

    displayConfig.algorithm_overlay =
        settings.value("display/algorithm_overlay", DisplayConfig::kDefaultOverlay).toBool();

    const QString s =
        settings
            .value("display/base_image",
                   QString::fromStdString(to_string(DisplayConfig::kDefaultImageBase)))
            .toString();

    displayConfig.image = ib_from_string(s.toStdString());

    displayConfig.mirrorMode =
        settings.value("display/mirror_mode", DisplayConfig::kDefaultOptions).toBool();

    displayConfig.smoothDisplay =
        settings.value("display/smooth_display", DisplayConfig::kDefaultOptions).toBool();
}

QDir ApplicationSettings::settingsDirectory()
{
    QSettings settings;
    QFileInfo info(settings.fileName());
    return info.dir();
}

bool ApplicationSettings::loadInitialPhi()
{
    bool isOk = false;

    QDir dir = settingsDirectory();
    QString phiPath = dir.filePath(kPhiInitFilename);

    QImage& img = imageSettings_.compute.initialPhi;

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

void ApplicationSettings::loadDefaultInitialPhi()
{
    const int width = 1280;
    const int height = 720;

    imageSettings_.compute.initialPhi = QImage(width, height, QImage::Format_Grayscale8);
    imageSettings_.compute.initialPhi.fill(Qt::black);

    QPainter painter(&imageSettings_.compute.initialPhi);

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);

    const int ellipseWidth = static_cast<int>(0.85 * width);
    const int ellipseHeight = static_cast<int>(0.85 * height);

    QRect boundingBox(width / 2 - ellipseWidth / 2, height / 2 - ellipseHeight / 2, ellipseWidth,
                      ellipseHeight);

    painter.drawEllipse(boundingBox);
}

bool ApplicationSettings::saveInitialPhi()
{
    bool isOk = false;

    QImage img = imageSettings_.compute.initialPhi;

    if (!img.isNull() && img.format() == QImage::Format_Grayscale8)
    {
        QDir dir = settingsDirectory();
        QString phiPath = dir.filePath(kPhiInitFilename);

        isOk = img.save(phiPath, "PNG");
    }

    return isOk;
}

const ImageSessionSettings& ApplicationSettings::imageSettings() const
{
    return imageSettings_;
}

const VideoSessionSettings& ApplicationSettings::videoSettings() const
{
    return videoSettings_;
}

Language ApplicationSettings::appLanguage() const
{
    return appLanguage_;
}

void ApplicationSettings::setAppLanguage(Language language)
{
    appLanguage_ = language;
}

} // namespace fluvel_app
