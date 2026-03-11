// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour_worker.hpp"

#include <QObject>
#include <QImage>

namespace fluvel_app
{

class ImageController : public QObject
{
    Q_OBJECT

public:
    ImageController(QObject* parent);

public slots:
    void loadImage(const QString& path);
    void onProcessedImageReady(const QImage& processed);
    void onContourUpdated(const fluvel_ip::ExportedContour& l_out,
                          const fluvel_ip::ExportedContour& l_in);
    void onStateChanged(fluvel_app::WorkerState state);
    void onDiagnosticsUpdated(const fluvel_ip::ContourDiagnostics& diag);

    void restart();
    void togglePause();
    void step();
    void converge();

signals:
    void errorOccurred(const QString& msg);
    void imageOpened(const QString& path);

    void inputImageReady(const QImage& inputImage);
    void displayedImageReady(const QImage& displayed);

    void contourUpdated(const QVector<QPointF>& l_out, const QVector<QPointF>& l_in);
    void stateChanged(fluvel_app::WorkerState state);

    void textDiagnosticsUpdated(QString string);

    void clearOverlaysRequested();

private:
    void onImgSettingsChanged(const ImageSessionSettings& conf);
    void onImgDisplaySettingsChanged(const DisplayConfig& display);

    void downscaleImage();
    void reinitializeWorker();
    void refreshView();

    QImage inputImage_;
    QImage downscaledImage_;
    QImage processedImage_;

    DisplayConfig displayConfig_;
    ImageComputeConfig computeConfig_;

    ActiveContourWorker acWorker_;
};

bool isSupportedImage(const QString& path);

} // namespace fluvel_app
