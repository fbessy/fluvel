// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "image_settings_controller.hpp"

#include <QCoreApplication>

namespace fluvel_app
{

ImageSettingsController::ImageSettingsController(const DownscaleConfig& downscaleConfig,
                                                 const ProcessingConfig& processingConfig,
                                                 fluvel_ip::Connectivity connectivity,
                                                 QObject* parent)
    : QObject(parent)
    , editedDownscaleConfig_(downscaleConfig)
    , editedProcessingConfig_(processingConfig)
{
    phiEditor_ = std::make_unique<PhiEditor>(
        ApplicationSettings::instance().imageSettings().compute.initialPhi);
    phiViewModel_ = std::make_unique<PhiViewModel>(phiEditor_.get(), connectivity);

    connect(phiViewModel_.get(), &PhiViewModel::viewChanged, this,
            &ImageSettingsController::onViewChanged);
}

void ImageSettingsController::updateEditedConfig(const DownscaleConfig& downscaleConfig,
                                                 const ProcessingConfig& processingConfig)
{
    editedDownscaleConfig_ = downscaleConfig;
    editedProcessingConfig_ = processingConfig;

    applyDownscale();
    applyProcessing();

    if (!processed_.isNull())
        phiViewModel_->setBackground(processed_);
}

ShapeInfo ImageSettingsController::computeShapeInfo(const UiShapeInfo& uiShape,
                                                    const QSize& targetSize)
{
    ShapeInfo info;

    info.type = uiShape.shape;

    const float canvasWidth = static_cast<float>(targetSize.width());
    const float canvasHeight = static_cast<float>(targetSize.height());

    float centerXPercent = static_cast<float>(uiShape.x) / 100.f;
    float centerYPercent = static_cast<float>(uiShape.y) / 100.f;

    float centerX = (centerXPercent + 0.5f) * canvasWidth;
    float centerY = (centerYPercent + 0.5f) * canvasHeight;

    float width = static_cast<float>(uiShape.width) / 100.f * canvasWidth;
    float height = static_cast<float>(uiShape.height) / 100.f * canvasHeight;

    float topLeftX = centerX - width / 2.f;
    float topLeftY = centerY - height / 2.f;

    info.boundingBox = QRect(static_cast<int>(topLeftX), static_cast<int>(topLeftY),
                             static_cast<int>(width), static_cast<int>(height));

    return info;
}

void ImageSettingsController::addShape(UiShapeInfo uiShape)
{
    if (!initializationEnabled_)
        return;

    if (!phiEditor_)
        return;

    if (phiEditor_->phi().isNull())
        return;

    auto shape = computeShapeInfo(uiShape, phiEditor_->phi().size());

    phiEditor_->addShape(shape);
}

void ImageSettingsController::subtractShape(UiShapeInfo uiShape)
{
    if (!initializationEnabled_)
        return;

    if (!phiEditor_)
        return;

    if (phiEditor_->phi().isNull())
        return;

    auto shape = computeShapeInfo(uiShape, phiEditor_->phi().size());

    phiEditor_->subtractShape(shape);
}

void ImageSettingsController::clearPhi()
{
    if (phiEditor_)
        phiEditor_->clear();
}

void ImageSettingsController::onInputImageReady(const QImage& input)
{
    input_ = input;

    applyDownscale();
    applyProcessing();

    phiEditor_->setSize(input_.size());
    phiViewModel_->setBackground(processed_);
}

void ImageSettingsController::onViewChanged(const QImage& imageSettings)
{
    if (!imageSettings.isNull())
        emit viewChanged(imageSettings);
}

void ImageSettingsController::onConnectivityChanged(fluvel_ip::Connectivity c)
{
    if (!phiViewModel_)
        return;

    phiViewModel_->setConnectivity(c);
}

void ImageSettingsController::onUpdateOverlay(UiShapeInfo uiShape)
{
    if (!initializationEnabled_ || !phiViewModel_)
        return;

    if (phiViewModel_->phi().isNull())
        return;

    auto shape = computeShapeInfo(uiShape, phiViewModel_->phi().size());

    phiViewModel_->setOverlay(shape);
}

QImage ImageSettingsController::commit()
{
    QImage initialPhi;

    if (phiEditor_)
        initialPhi = phiEditor_->commit();

    return initialPhi;
}

void ImageSettingsController::revert()
{
    if (phiEditor_)
        phiEditor_->revert();
}

void ImageSettingsController::applyDownscale()
{
    if (input_.isNull())
        return;

    if (input_.format() != QImage::Format_Grayscale8 && input_.format() != QImage::Format_RGB32)
        return;

    if (editedDownscaleConfig_.hasDownscale)
    {
        const int df = editedDownscaleConfig_.downscaleFactor;

        assert(df == 2 || df == 4);

        downscaled_ = input_.scaled(input_.width() / df, input_.height() / df,
                                    Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    else
    {
        downscaled_ = input_;
    }
}

void ImageSettingsController::applyProcessing()
{
    if (downscaled_.isNull())
        return;

    processed_ = downscaled_;

    QImage img;

    int channelsNbr = 3;

    if (downscaled_.format() == QImage::Format_Grayscale8)
    {
        img = downscaled_;
        channelsNbr = 1;
    }
    else if (downscaled_.format() == QImage::Format_RGB32)
    {
        img = downscaled_.convertToFormat(QImage::Format_RGB888);
        channelsNbr = 3;
    }

    if (img.isNull())
        return;

    const int width = img.width();

    if (img.bytesPerLine() != static_cast<qsizetype>(width * channelsNbr))
        return;

    const qsizetype stride = img.bytesPerLine();

    const int bytesPerPixel = static_cast<int>(stride / width);

    fluvel_ip::Filters filters(img.constBits(), width, img.height(), bytesPerPixel);

    const auto& fc = editedProcessingConfig_;

    emit processingStarted();
    QCoreApplication::processEvents();

    clock_type::time_point measurementStartTime = clock_type::now();

    if (fc.hasProcessing())
    {
        if (fc.has_gaussian_noise)
        {
            filters.gaussian_white_noise(fc.std_noise);
        }
        if (fc.has_salt_noise)
        {
            filters.impulsive_noise(fc.proba_noise);
        }
        if (fc.has_speckle_noise)
        {
            filters.speckle(fc.std_speckle_noise);
        }

        if (fc.has_mean_filt)
        {
            filters.mean_filtering(fc.kernel_mean_length);
        }
        if (fc.has_gaussian_filt)
        {
            filters.gaussian_filtering(fc.kernel_gaussian_length, fc.sigma);
        }

        if (fc.has_median_filt)
        {
            if (fc.has_O1_algo)
            {
                filters.median_filtering_o1(fc.kernel_median_length);
            }
            else
            {
                filters.median_filtering_oNlogN(fc.kernel_median_length);
            }
        }
        if (fc.has_aniso_diff)
        {
            filters.anisotropic_diffusion(fc.max_itera, fc.lambda, fc.kappa, fc.aniso_option);
        }

        if (fc.has_open_filt)
        {
            if (fc.has_O1_morpho)
            {
                filters.opening_o1(fc.kernel_open_length);
            }
            else
            {
                filters.opening(fc.kernel_open_length);
            }
        }

        if (fc.has_close_filt)
        {
            if (fc.has_O1_morpho)
            {
                filters.closing_o1(fc.kernel_close_length);
            }
            else
            {
                filters.closing(fc.kernel_close_length);
            }
        }

        if (fc.has_top_hat_filt)
        {
            if (fc.is_white_top_hat)
            {
                if (fc.has_O1_morpho)
                {
                    filters.white_top_hat_o1(fc.kernel_tophat_length);
                }
                else
                {
                    filters.white_top_hat(fc.kernel_tophat_length);
                }
            }
            else
            {
                if (fc.has_O1_morpho)
                {
                    filters.black_top_hat_o1(fc.kernel_tophat_length);
                }
                else
                {
                    filters.black_top_hat(fc.kernel_tophat_length);
                }
            }
        }

        auto endTime = clock_type::now();
        double elapsedSec = std::chrono::duration<double>(endTime - measurementStartTime).count();

        emit filterPipelineProcessed(elapsedSec);

        if (img.format() == QImage::Format_RGB888)
        {
            processed_ =
                QImage(filters.get_filtered(), img.width(), img.height(), QImage::Format_RGB888)
                    .convertToFormat(QImage::Format_RGB32);
        }
        else if (img.format() == QImage::Format_Grayscale8)
        {
            processed_ =
                QImage(filters.get_filtered(), img.width(), img.height(), QImage::Format_Grayscale8)
                    .copy();
        }
    }

    if (processed_.isNull())
        processed_ = downscaled_;
}

void ImageSettingsController::setInteractiveMode(bool enabled)
{
    if (!phiViewModel_)
        return;

    initializationEnabled_ = enabled;

    phiViewModel_->setInteractiveMode(enabled);

    if (enabled)
        phiViewModel_->showOverlay();
    else
        phiViewModel_->hideOverlay();
}

} // namespace fluvel_app
