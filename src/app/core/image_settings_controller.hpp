// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "phi_view_model.hpp"
#include "phi_editor.hpp"

#include <QObject>

namespace fluvel_app
{

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
    ImageSettingsController(QObject* parent);

    void addShape(UiShapeInfo uiShape);
    void subtractShape(UiShapeInfo uiShape);
    void clearPhi();
    void onInputImageReady(const QImage& inputImage);

    void accept();
    void reject();

public slots:
    void onUpdateOverlay(fluvel_app::UiShapeInfo uiShape);
    void onViewChanged(const QImage& imageSettings);

signals:
    void viewChanged(const QImage& imageSettings);

private:
    void setInitialPhi(const QImage& phi);

    ShapeInfo computeShapeInfo(const UiShapeInfo& uiShape);

    std::unique_ptr<PhiEditor> phiEditor_;
    std::unique_ptr<PhiViewModel> phiViewModel_;
};

} // namespace fluvel_app
