// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "application_settings.hpp"
#include "phi_editor.hpp"
#include "phi_view_model.hpp"

#include <QObject>

#include <chrono>

namespace fluvel_app
{

using clock_type = std::chrono::steady_clock;

struct UiShapeInfo
{
    ShapeType shape;
    int width;
    int height;
    int x;
    int y;
};

class ImageSettingsController : public QObject
{
    Q_OBJECT

public:
    ImageSettingsController(const DownscaleConfig& downscaleConfig,
                            const ProcessingConfig& processingConfig, QObject* parent);

    void addShape(UiShapeInfo uiShape);
    void subtractShape(UiShapeInfo uiShape);
    void clearPhi();
    void onInputImageReady(const QImage& inputImage);

    void updateEditedConfig(const DownscaleConfig& downscaleConfig,
                            const ProcessingConfig& processingConfig);

    void accept();
    void reject();

public slots:
    void onUpdateOverlay(fluvel_app::UiShapeInfo uiShape);
    void onViewChanged(const QImage& imageSettings);
    void onConnectivityChanged(fluvel_ip::Connectivity c);

signals:
    void viewChanged(const QImage& imageSettings);
    void processingStarted();
    void filterPipelineProcessed(double elapsedSec);

private:
    void setInitialPhi(const QImage& phi);

    ShapeInfo computeShapeInfo(const UiShapeInfo& uiShape);

    void applyDownscale();
    void applyProcessing();

    std::unique_ptr<PhiEditor> phiEditor_;
    std::unique_ptr<PhiViewModel> phiViewModel_;

    DownscaleConfig editedDownscaleConfig_;
    ProcessingConfig editedProcessingConfig_;

    QImage input_;
    QImage downscaled_;
    QImage processed_;
};

} // namespace fluvel_app
