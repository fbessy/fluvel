// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "application_settings_types.hpp"
#include "phi_editor.hpp"
#include "phi_view_model.hpp"

#include <QObject>

namespace fluvel_app
{

// normalized shape in function of the image size
struct UiShapeInfo
{
    ShapeType shape;
    int width;  // percent
    int height; // percent
    int x;      // percent centered
    int y;      // percent centered
};

class ImageSettingsController : public QObject
{
    Q_OBJECT

public:
    ImageSettingsController(const ImageSessionSettings& session, QObject* parent);

    void setViewVisible(bool v);

    void addShape(UiShapeInfo uiShape);
    void subtractShape(UiShapeInfo uiShape);
    void clearPhi();
    void onInputImageReady(const QImage& inputImage);

    void updateEditedConfig(const DownscaleParams& downscaleParams,
                            const fluvel_ip::ProcessingParams& processingParams);

    QImage commit();
    void revert();

    void onUpdateOverlay(fluvel_app::UiShapeInfo uiShape);
    void onConnectivityChanged(fluvel_ip::Connectivity c);
    void setInteractiveMode(bool enabled);

signals:
    void viewChanged(const QImage& imageSettings);
    void processingStarted();
    void filterPipelineProcessed(double elapsedSec);

private:
    void onViewChanged(const QImage& imageSettings);

    ShapeInfo computeShapeInfo(const UiShapeInfo& uiShape, const QSize& targetSize);

    void applyDownscale();
    void applyProcessing();

    void refreshPreview();

    std::unique_ptr<PhiEditor> phiEditor_;
    std::unique_ptr<PhiViewModel> phiViewModel_;

    DownscaleParams editedDownscaleParams_;
    fluvel_ip::ProcessingParams editedProcessingParams_;

    QImage input_;
    QImage downscaled_;
    QImage processed_;

    bool initializationEnabled_{false};

    bool needsRefresh_{false};
    bool viewVisible_{false};
};

} // namespace fluvel_app
