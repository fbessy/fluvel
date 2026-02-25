// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "image_settings_controller.hpp"

#include "application_settings.hpp"

namespace ofeli_app
{

ImageSettingsController::ImageSettingsController(QObject* parent)
    : QObject(parent)
{
    phiEditor_ = std::make_unique<PhiEditor>(AppSettings::instance().imgConfig.compute.initialPhi);
    phiViewModel_ = std::make_unique<PhiViewModel>(phiEditor_.get());

    connect(phiEditor_.get(), &PhiEditor::phiAccepted, this,
            &ImageSettingsController::setInitialPhi);

    connect(phiViewModel_.get(), &PhiViewModel::viewChanged, this,
            &ImageSettingsController::onViewChanged);

    // connect(imageController,    &ImageController::imageReadyWithoutResize,
    //         phiViewModel.get(), &PhiViewModel::setBackgroundWithUpdate);

    // connect(imageController,
    //         &ImageController::imageReadyWithResize,
    //         phiEditor.get(),
    //         [this](const QImage &img)
    //         {
    //             phiViewModel->setBackground(img);
    //             phiEditor->onImageSizeReady(img.width(), img.height());
    //         });
}

ShapeInfo ImageSettingsController::computeShapeInfo(const UiShapeInfo& uiShape)
{
    ShapeInfo info;

    // --- Type de shape ---
    info.type = uiShape.shape;

    const int canvasWidth = phiEditor_->phi().width();
    const int canvasHeight = phiEditor_->phi().height();

    // Récupération des valeurs des sliders (en pourcentage)
    float centerXPercent =
        static_cast<float>(uiShape.x) / 100.0f; // De -500 à +500, donc normalisé autour de 0
    float centerYPercent = static_cast<float>(uiShape.y) / 100.0f;

    // Calcul de la position du centre en pixels
    float centerX = (centerXPercent + 0.5f) * static_cast<float>(canvasWidth);
    float centerY = (centerYPercent + 0.5f) * static_cast<float>(canvasHeight);

    // Récupération des dimensions de la shape en pixels
    float width = static_cast<float>(uiShape.width) / 100.0f * static_cast<float>(canvasWidth);

    float height = static_cast<float>(uiShape.height) / 100.0f * static_cast<float>(canvasHeight);

    // Calcul de la bounding box à partir du centre et des dimensions
    float topLeftX = centerX - (width / 2.f);
    float topLeftY = centerY - (height / 2.f);

    info.boundingBox = QRect(static_cast<int>(topLeftX), static_cast<int>(topLeftY),
                             static_cast<int>(width), static_cast<int>(height));

    return info;
}

void ImageSettingsController::addShape(UiShapeInfo uiShape)
{
    if (!phiEditor_)
        return;

    auto shape = computeShapeInfo(uiShape);

    phiEditor_->addShape(shape);
}

void ImageSettingsController::subtractShape(UiShapeInfo uiShape)
{
    if (!phiEditor_)
        return;

    auto shape = computeShapeInfo(uiShape);

    phiEditor_->subtractShape(shape);
}

void ImageSettingsController::clearPhi()
{
    if (phiEditor_)
        phiEditor_->clear();
}

void ImageSettingsController::onInputImageReady(const QImage& inputImage)
{
    phiEditor_->setSize(inputImage.size());
    phiViewModel_->setBackground(inputImage);
}

void ImageSettingsController::onViewChanged(const QImage& imageSettings)
{
    if (!imageSettings.isNull())
        emit viewChanged(imageSettings);
}

void ImageSettingsController::onUpdateOverlay(UiShapeInfo uiShape)
{
    if (!phiViewModel_)
        return;

    auto shape = computeShapeInfo(uiShape);

    phiViewModel_->setOverlay(shape);
}

void ImageSettingsController::accept()
{
    if (phiEditor_)
        phiEditor_->accept();
}
void ImageSettingsController::reject()
{
    if (phiEditor_)
        phiEditor_->reject();
}

void ImageSettingsController::setInitialPhi(const QImage& phi)
{
    AppSettings::instance().imgConfig.compute.initialPhi = phi;
}

} // namespace ofeli_app
