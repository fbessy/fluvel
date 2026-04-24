// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour_worker.hpp"
#include "application_settings_types.hpp"

#include <QImage>
#include <QObject>
#include <QString>
#include <QVector>

namespace fluvel_app
{

class ImageController : public QObject
{
    Q_OBJECT

public:
    ImageController(const ImageSessionSettings& session, QObject* parent);

    void loadImage(const QString& path);

    void onImageSettingsChanged(const fluvel_app::ImageSessionSettings& session);
    void onImageDisplaySettingsChanged(const fluvel_app::DisplayConfig& display);

    void restart();
    void togglePause();
    void step();
    void converge();

signals:
    void errorOccurred(const QString& msg);
    void imageOpened(const QString& path);

    void inputImageReady(const QImage& inputImage);
    void displayedImageReady(const QImage& displayed);

    void contourUpdated(const QVector<QPointF>& outerContour, const QVector<QPointF>& innerContour);
    void stateChanged(fluvel_app::WorkerState state);

    void textDiagnosticsUpdated(QString string);

    void clearContourRequested();

private:
    void onProcessedImageReady(const QImage& processed);
    void onContourUpdated(const fluvel_ip::ExportedContour& outerContour,
                          const fluvel_ip::ExportedContour& innerContour);
    void onStateChanged(fluvel_app::WorkerState state);
    void onDiagnosticsUpdated(const fluvel_ip::ContourDiagnostics& diag);

    void downscaleImage();
    void reinitializeWorker();
    void refreshView();

    QImage inputImage_;
    QImage downscaledImage_;
    QImage processedImage_;

    DisplayConfig displayConfig_;
    ImageComputeConfig computeConfig_;

    ActiveContourWorker activeContourWorker_;
};

} // namespace fluvel_app
