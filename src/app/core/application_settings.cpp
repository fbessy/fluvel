// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "application_settings.hpp"

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

    saveAlgo(Session::Image, imageSettings_.compute.contourConfig);

    // phi init used only for image session
    saveInitialPhi();

    saveDownscale(Session::Image, imageSettings_.compute.downscale);

    settings.setValue("preprocess/processing_enabled",
                      imageSettings_.compute.processing.processingEnabled);

    settings.setValue("preprocess/gaussian_noise_enabled",
                      imageSettings_.compute.processing.gaussianNoiseEnabled);
    settings.setValue("preprocess/noise_std_dev", imageSettings_.compute.processing.noiseStdDev);
    settings.setValue("preprocess/salt_noise_enabled",
                      imageSettings_.compute.processing.saltNoiseEnabled);
    settings.setValue("preprocess/salt_noise_probability",
                      imageSettings_.compute.processing.saltNoiseProbability);
    settings.setValue("preprocess/speckle_noise_enabled",
                      imageSettings_.compute.processing.speckleNoiseEnabled);
    settings.setValue("preprocess/speckle_noise_std_dev",
                      imageSettings_.compute.processing.speckleNoiseStdDev);

    settings.setValue("preprocess/median_filter_enabled",
                      imageSettings_.compute.processing.medianFilterEnabled);
    settings.setValue("preprocess/median_kernel_size",
                      imageSettings_.compute.processing.medianKernelSize);

    settings.setValue("preprocess/mean_filter_enabled",
                      imageSettings_.compute.processing.meanFilterEnabled);
    settings.setValue("preprocess/mean_kernel_size",
                      imageSettings_.compute.processing.meanKernelSize);

    settings.setValue("preprocess/anisotropic_diffusion_enabled",
                      imageSettings_.compute.processing.anisotropicDiffusionEnabled);
    settings.setValue("preprocess/conduction_function",
                      static_cast<int>(imageSettings_.compute.processing.conductionFunction));
    settings.setValue("preprocess/max_iterations", imageSettings_.compute.processing.maxIterations);
    settings.setValue("preprocess/lambda", imageSettings_.compute.processing.lambda);
    settings.setValue("preprocess/kappa", imageSettings_.compute.processing.kappa);

    settings.setValue("preprocess/opening_enabled",
                      imageSettings_.compute.processing.openingEnabled);
    settings.setValue("preprocess/opening_kernel_size",
                      imageSettings_.compute.processing.openingKernelSize);

    settings.setValue("preprocess/closing_enabled",
                      imageSettings_.compute.processing.closingEnabled);
    settings.setValue("preprocess/closing_kernel_size",
                      imageSettings_.compute.processing.closingKernelSize);

    settings.setValue("preprocess/top_hat_enabled",
                      imageSettings_.compute.processing.topHatEnabled);
    settings.setValue("preprocess/use_white_top_hat",
                      imageSettings_.compute.processing.useWhiteTopHat);
    settings.setValue("preprocess/top_hat_kernel_size",
                      imageSettings_.compute.processing.topHatKernelSize);

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

    // preprocess

    saveDownscale(Session::Camera, videoSettings_.compute.downscale);

    settings.setValue("preprocess/spatial_filtering_enabled",
                      videoSettings_.compute.spatialFilteringEnabled);

    settings.setValue("preprocess/temporal_filtering_enabled",
                      videoSettings_.compute.temporalFilteringEnabled);

    // algo

    saveAlgo(Session::Camera, videoSettings_.compute.contourConfig);

    // display

    saveDisplay(Session::Camera, videoSettings_.display);

    emit videoSettingsChanged(videoSettings_);
}

void ApplicationSettings::setVideoSessionSettings(const VideoSessionSettings& config)
{
    videoSettings_ = config;

    saveVideoSessionSettings();
}

void ApplicationSettings::saveAlgo(Session session, const ActiveContourConfig& algoConfig)
{
    QSettings settings = sessionSettings(session);

    settings.setValue("algo/connectivity", fluvel_ip::to_string(algoConfig.connectivity));

    settings.setValue("algo/cycle1_iterations", algoConfig.contourParams.Na);

    settings.setValue("algo/cycle2_smoothing_enabled", algoConfig.contourParams.cycle2Enabled);

    settings.setValue("algo/cycle2_iterations", algoConfig.contourParams.Ns);

    settings.setValue("algo/disk_radius", algoConfig.contourParams.diskRadius);

    settings.setValue("algo/lambda_out", algoConfig.regionParams.lambdaOut);

    settings.setValue("algo/lambda_in", algoConfig.regionParams.lambdaIn);

    settings.setValue("algo/color_space", fluvel_ip::to_string(algoConfig.regionParams.colorSpace));

    settings.setValue("algo/color_weight_w1", algoConfig.regionParams.weights.c1);

    settings.setValue("algo/color_weight_w2", algoConfig.regionParams.weights.c2);

    settings.setValue("algo/color_weight_w3", algoConfig.regionParams.weights.c3);
}

void ApplicationSettings::saveDownscale(Session session, const DownscaleParams& downscaleParams)
{
    QSettings settings = sessionSettings(session);

    settings.setValue("preprocess/downscale_enabled", downscaleParams.downscaleEnabled);

    settings.setValue("preprocess/downscale_factor", downscaleParams.downscaleFactor);
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

    settings.setValue("display/outer_contour/visible", displayConfig.outerContourVisible);
    settings.setValue("display/outer_contour/rgb", rgbToString(displayConfig.outerContourColor));

    settings.setValue("display/inner_contour/visible", displayConfig.innerContourVisible);
    settings.setValue("display/inner_contour/rgb", rgbToString(displayConfig.innerContourColor));

    settings.setValue("display/algorithm_overlay/enabled", displayConfig.algorithmOverlayEnabled);

    settings.setValue("display/mode", to_string(displayConfig.displayMode));

    settings.setValue("display/mirror_mode", displayConfig.mirrorMode);

    settings.setValue("display/smooth_display", displayConfig.smoothDisplay);
}

void ApplicationSettings::loadImageSessionSettings()
{
    QSettings settings = imgSessionSettings();

    loadAlgo(Session::Image, imageSettings_.compute.contourConfig);

    bool isOk = loadInitialPhi();

    if (!isOk)
    {
        loadDefaultInitialPhi();
    }

    // preprocess
    loadDownscale(Session::Image, imageSettings_.compute.downscale);

    auto& fc = imageSettings_.compute.processing;

    fc.processingEnabled = settings.value("preprocess/processing_enabled", false).toBool();

    fc.gaussianNoiseEnabled = settings
                                  .value("preprocess/gaussian_noise_enabled",
                                         fluvel_ip::ProcessingParams::kDefaultDisabled)
                                  .toBool();
    fc.noiseStdDev =
        settings.value("preprocess/noise_std_dev", fluvel_ip::ProcessingParams::kDefaultStdNoise)
            .toFloat();
    fc.saltNoiseEnabled =
        settings
            .value("preprocess/salt_noise_enabled", fluvel_ip::ProcessingParams::kDefaultDisabled)
            .toBool();
    fc.saltNoiseProbability = settings
                                  .value("preprocess/salt_noise_probability",
                                         fluvel_ip::ProcessingParams::kDefaultSaltNoise)
                                  .toFloat();
    fc.speckleNoiseEnabled = settings
                                 .value("preprocess/speckle_noise_enabled",
                                        fluvel_ip::ProcessingParams::kDefaultDisabled)
                                 .toBool();
    fc.speckleNoiseStdDev = settings
                                .value("preprocess/speckle_noise_std_dev",
                                       fluvel_ip::ProcessingParams::kDefaultSpeckleNoise)
                                .toFloat();

    fc.medianFilterEnabled = settings
                                 .value("preprocess/median_filter_enabled",
                                        fluvel_ip::ProcessingParams::kDefaultDisabled)
                                 .toBool();
    fc.medianKernelSize =
        settings
            .value("preprocess/median_kernel_size", fluvel_ip::ProcessingParams::kDefaultKernelSize)
            .toInt();

    fc.meanFilterEnabled =
        settings
            .value("preprocess/mean_filter_enabled", fluvel_ip::ProcessingParams::kDefaultDisabled)
            .toBool();
    fc.meanKernelSize =
        settings
            .value("preprocess/mean_kernel_size", fluvel_ip::ProcessingParams::kDefaultKernelSize)
            .toInt();

    fc.anisotropicDiffusionEnabled = settings
                                         .value("preprocess/anisotropic_diffusion_enabled",
                                                fluvel_ip::ProcessingParams::kDefaultDisabled)
                                         .toBool();

    fc.conductionFunction = static_cast<fluvel_ip::filter::ConductionFunction>(
        settings
            .value("preprocess/conduction_function",
                   static_cast<int>(fluvel_ip::ProcessingParams::kDefaultConductionFunction))
            .toInt());

    fc.maxIterations =
        settings
            .value("preprocess/max_iterations", fluvel_ip::ProcessingParams::kDefaultMaxIterations)
            .toInt();
    fc.lambda =
        settings.value("preprocess/lambda", fluvel_ip::ProcessingParams::kDefaultLambda).toDouble();
    fc.kappa =
        settings.value("preprocess/kappa", fluvel_ip::ProcessingParams::kDefaultKappa).toDouble();

    fc.openingEnabled =
        settings.value("preprocess/opening_enabled", fluvel_ip::ProcessingParams::kDefaultDisabled)
            .toBool();
    fc.openingKernelSize = settings
                               .value("preprocess/opening_kernel_size",
                                      fluvel_ip::ProcessingParams::kDefaultKernelSize)
                               .toInt();
    fc.closingEnabled =
        settings.value("preprocess/closing_enabled", fluvel_ip::ProcessingParams::kDefaultDisabled)
            .toBool();
    fc.closingKernelSize = settings
                               .value("preprocess/closing_kernel_size",
                                      fluvel_ip::ProcessingParams::kDefaultKernelSize)
                               .toInt();
    fc.topHatEnabled =
        settings.value("preprocess/top_hat_enabled", fluvel_ip::ProcessingParams::kDefaultDisabled)
            .toBool();
    fc.useWhiteTopHat =
        settings
            .value("preprocess/use_white_top_hat", fluvel_ip::ProcessingParams::kDefaultWhiteTopHat)
            .toBool();
    fc.topHatKernelSize = settings
                              .value("preprocess/top_hat_kernel_size",
                                     fluvel_ip::ProcessingParams::kDefaultKernelSize)
                              .toInt();

    // display
    loadDisplay(Session::Image, imageSettings_.display);
}

void ApplicationSettings::loadVideoSessionSettings()
{
    QSettings settings = camSessionSettings();

    // preprocess tab

    loadDownscale(Session::Camera, videoSettings_.compute.downscale);

    videoSettings_.compute.spatialFilteringEnabled =
        settings
            .value("preprocess/spatial_filtering_enabled",
                   VideoComputeConfig::kDefaultSpatialFilteringEnabled)
            .toBool();

    videoSettings_.compute.temporalFilteringEnabled =
        settings
            .value("preprocess/temporal_filtering_enabled",
                   VideoComputeConfig::kDefaultTemporalFilteringEnabled)
            .toBool();

    loadAlgo(Session::Camera, videoSettings_.compute.contourConfig);

    // display
    loadDisplay(Session::Camera, videoSettings_.display);
}

void ApplicationSettings::loadAlgo(Session session, ActiveContourConfig& algoConfig)
{
    QSettings settings = sessionSettings(session);

    const QString s = settings
                          .value("algo/connectivity",
                                 fluvel_ip::to_string(ActiveContourConfig::kDefaultConnectivity))
                          .toString();

    algoConfig.connectivity = fluvel_ip::connectivity_from_string(s.toStdString());

    algoConfig.contourParams.Na =
        settings.value("algo/cycle1_iterations", fluvel_ip::ActiveContourParams::kDefaultNa)
            .toInt();
    algoConfig.contourParams.cycle2Enabled =
        settings
            .value("algo/cycle2_smoothing_enabled",
                   fluvel_ip::ActiveContourParams::kDefaultIsCycle2)
            .toBool();
    algoConfig.contourParams.Ns =
        settings.value("algo/cycle2_iterations", fluvel_ip::ActiveContourParams::kDefaultNs)
            .toInt();
    algoConfig.contourParams.diskRadius =
        settings.value("algo/disk_radius", fluvel_ip::ActiveContourParams::kDefaultDiskRadius)
            .toInt();

    switch (session)
    {
        case Session::Camera:
            algoConfig.contourParams.failureMode = fluvel_ip::FailureHandlingMode::RecoverOnFailure;
            break;

        case Session::Image:
        default:
            algoConfig.contourParams.failureMode = fluvel_ip::FailureHandlingMode::StopOnFailure;
            break;
    }

    algoConfig.regionParams.lambdaOut =
        settings.value("algo/lambda_out", fluvel_ip::RegionParams::kDefaultLambdaOut).toInt();
    algoConfig.regionParams.lambdaIn =
        settings.value("algo/lambda_in", fluvel_ip::RegionParams::kDefaultLambdaIn).toInt();

    const QString s_cs =
        settings
            .value("algo/color_space",
                   fluvel_ip::to_string(fluvel_ip::RegionColorParams::kDefaultColorSpace))
            .toString();

    algoConfig.regionParams.colorSpace = fluvel_ip::colorSpaceFromString(s_cs.toStdString());

    algoConfig.regionParams.weights.c1 =
        settings.value("algo/color_weight_w1", fluvel_ip::RegionColorParams::kDefaultWeights.c1)
            .toInt();
    algoConfig.regionParams.weights.c2 =
        settings.value("algo/color_weight_w2", fluvel_ip::RegionColorParams::kDefaultWeights.c2)
            .toInt();
    algoConfig.regionParams.weights.c3 =
        settings.value("algo/color_weight_w3", fluvel_ip::RegionColorParams::kDefaultWeights.c3)
            .toInt();
}

void ApplicationSettings::loadDownscale(Session session, DownscaleParams& downscaleParams)
{
    QSettings settings = sessionSettings(session);

    bool defaultIsDownscale = (session == Session::Camera);

    downscaleParams.downscaleEnabled =
        settings.value("preprocess/downscale_enabled", defaultIsDownscale).toBool();

    downscaleParams.downscaleFactor =
        settings.value("preprocess/downscale_factor", DownscaleParams::kDefaultDownscaleFactor)
            .toInt();
}
void ApplicationSettings::loadDisplay(Session session, DisplayConfig& displayConfig)
{
    QSettings settings = sessionSettings(session);

    displayConfig.outerContourVisible =
        settings.value("display/outer_contour/visible", DisplayConfig::kDefaultContourVisible)
            .toBool();
    displayConfig.outerContourColor =
        rgbFromString(settings
                          .value("display/outer_contour/rgb",
                                 rgbToString(DisplayConfig::kDefaultOuterContourColor))
                          .toString(),
                      DisplayConfig::kDefaultOuterContourColor);

    displayConfig.innerContourVisible =
        settings.value("display/inner_contour/visible", DisplayConfig::kDefaultContourVisible)
            .toBool();
    displayConfig.innerContourColor =
        rgbFromString(settings
                          .value("display/inner_contour/rgb",
                                 rgbToString(DisplayConfig::kDefaultInnerContourColor))
                          .toString(),
                      DisplayConfig::kDefaultInnerContourColor);

    displayConfig.algorithmOverlayEnabled =
        settings.value("display/algorithm_overlay/enabled", DisplayConfig::kDefaultOverlayEnabled)
            .toBool();

    const QString s =
        settings
            .value("display/mode",
                   QString::fromStdString(to_string(DisplayConfig::kDefaultDisplayMode)))
            .toString();

    displayConfig.displayMode = imageDisplayModeFromString(s.toStdString());

    displayConfig.mirrorMode =
        settings.value("display/mirror_mode", DisplayConfig::kDefaultOptionDisabled).toBool();

    displayConfig.smoothDisplay =
        settings.value("display/smooth_display", DisplayConfig::kDefaultOptionDisabled).toBool();
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
