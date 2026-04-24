// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "image_settings_controller.hpp"
#include "elapsed_timer.hpp"
#include "image_adapters.hpp"
#include "image_pipeline.hpp"

#include <QCoreApplication>

#ifdef FLUVEL_DEBUG
#include <QDebug>
#endif

namespace fluvel_app
{

ImageSettingsController::ImageSettingsController(const ImageSessionSettings& session,
                                                 QObject* parent)
    : QObject(parent)
{
    const auto& sc = session.compute;
    editedDownscaleConfig_ = sc.downscale;
    editedProcessingConfig_ = sc.processing;

    phiEditor_ = std::make_unique<PhiEditor>(sc.initialPhi);

    phiViewModel_ = std::make_unique<PhiViewModel>(phiEditor_.get(), sc.contourConfig.connectivity);

    connect(phiViewModel_.get(), &PhiViewModel::viewChanged, this,
            &ImageSettingsController::onViewChanged);
}

void ImageSettingsController::updateEditedConfig(
    const DownscaleConfig& downscaleConfig, const fluvel_ip::ProcessingConfig& processingConfig)
{
    editedDownscaleConfig_ = downscaleConfig;
    editedProcessingConfig_ = processingConfig;

    refreshPreview();
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

    if (!viewVisible_)
    {
        needsRefresh_ = true;
        return;
    }

    phiEditor_->setSize(input_.size());

    refreshPreview();
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
    downscaled_ = input_;

    if (input_.isNull())
        return;

    if (input_.format() != QImage::Format_Grayscale8 && input_.format() != QImage::Format_RGB32)
        return;

    if (editedDownscaleConfig_.downscaleEnabled)
    {
        const int df = editedDownscaleConfig_.downscaleFactor;

        assert(df == 2 || df == 4);

        downscaled_ = input_.scaled(input_.width() / df, input_.height() / df,
                                    Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
}

void ImageSettingsController::applyProcessing()
{
    processed_ = downscaled_;

    if (downscaled_.isNull())
        return;

#ifdef FLUVEL_DEBUG
    qDebug() << __FILE__ << " applyProcessing() " << __LINE__ << __func__;
#endif

    fluvel_ip::ImageView img = imageViewFromQImage(downscaled_);
    fluvel_ip::ImagePipeline filter;

    filter.reset(img);

    emit processingStarted();
    QCoreApplication::processEvents();

    fluvel_ip::ElapsedTimer measurementTimer;
    measurementTimer.start();

    filter.apply(img, editedProcessingConfig_);

    double elapsedSec = measurementTimer.elapsedSec();

    emit filterPipelineProcessed(elapsedSec);

    processed_ = toQImageCopy(filter.outputView());

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

void ImageSettingsController::refreshPreview()
{
    applyDownscale();
    applyProcessing();

    if (!processed_.isNull())
        phiViewModel_->setBackground(processed_);
}

void ImageSettingsController::setViewVisible(bool v)
{
    viewVisible_ = v;

    if (viewVisible_ && needsRefresh_)
    {
        needsRefresh_ = false;
        refreshPreview();
    }
}

} // namespace fluvel_app
