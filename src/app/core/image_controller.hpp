// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "active_contour_worker.hpp"

#include <QImage>
#include <QObject>

namespace ofeli_app
{

class ImageController : public QObject
{
    Q_OBJECT

public:
    ImageController(QObject* parent);

public slots:
    void loadImage(const QString& path);
    void onProcessedImageReady(const QImage& processed);
    void onContourUpdated(const ofeli_ip::ExportedContour& l_out,
                          const ofeli_ip::ExportedContour& l_in);
    void onStateChanged(ofeli_app::WorkerState state);

    void restart();
    void togglePause();
    void step();
    void converge();

signals:
    void inputImageReady(const QImage& inputImage);
    void displayedImageReady(const QImage& displayed);

    void contourUpdated(const QVector<QPointF>& l_out, const QVector<QPointF>& l_in);
    void stateChanged(ofeli_app::WorkerState state);

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

    QImage originalInitialPhi_;

    DisplayConfig displayConfig_;
    ImageComputeConfig computeConfig_;

    ActiveContourWorker acWorker_;
};

} // namespace ofeli_app
