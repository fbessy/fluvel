// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "image_controller.hpp"
#include "contour_adapters.hpp"
#include "file_utils.hpp"

#include <QImageReader>

#ifdef FLUVEL_DEBUG
#include "image_debug.hpp"
#endif

namespace fluvel_app
{

ImageController::ImageController(const ImageSessionSettings& session, QObject* parent)
    : QObject(parent)
{
    onImageSettingsChanged(session);
    onImageDisplaySettingsChanged(session.display);

    connect(&activeContourWorker_, &ActiveContourWorker::processedImageReady, this,
            &ImageController::onProcessedImageReady);

    connect(&activeContourWorker_, &ActiveContourWorker::contourUpdated, this,
            &ImageController::onContourUpdated, Qt::QueuedConnection);

    connect(&activeContourWorker_, &ActiveContourWorker::stateChanged, this,
            &ImageController::onStateChanged);

    connect(&activeContourWorker_, &ActiveContourWorker::diagnosticsUpdated, this,
            &ImageController::onDiagnosticsUpdated);
}

void ImageController::loadImage(const QString& path)
{
    if (path.isEmpty())
        return;

    if (!file_utils::isSupportedImage(path))
    {
        emit errorOccurred(tr("Unsupported image format."));
        return;
    }

    QImageReader reader(path);
    QImage img = reader.read();

    if (img.isNull())
    {
        emit errorOccurred(tr("Failed to load image."));
        return;
    }

    inputImage_ = img;

    if (inputImage_.isGrayscale())
        inputImage_ = inputImage_.convertToFormat(QImage::Format_Grayscale8);
    else
        inputImage_ = inputImage_.convertToFormat(QImage::Format_RGB32);

    reinitializeWorker();

    emit inputImageReady(inputImage_);
    emit imageOpened(path);
}

void ImageController::downscaleImage()
{
    if (inputImage_.isNull())
        return;

    if (inputImage_.format() != QImage::Format_Grayscale8 &&
        inputImage_.format() != QImage::Format_RGB32)
        return;

    if (computeConfig_.downscale.hasDownscale)
    {
        const int df = computeConfig_.downscale.downscaleFactor;

        assert(df == 2 || df == 4);

        downscaledImage_ = inputImage_.scaled(inputImage_.width() / df, inputImage_.height() / df,
                                              Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    else
    {
        downscaledImage_ = inputImage_;
    }
}

void ImageController::reinitializeWorker()
{
    if (inputImage_.isNull())
        return;

    downscaleImage();

    if (downscaledImage_.isNull())
        return;

    if (computeConfig_.initialPhi.isNull())
        return;

    ImageComputeConfig config = computeConfig_;

#ifdef FLUVEL_DEBUG
    qDebug() << __FILE__ << ":" << __LINE__ << __func__
             << "phi:" << image_debug::describeImage(config.initialPhi);
#endif

    if (computeConfig_.initialPhi.size() != downscaledImage_.size())
    {
        config.initialPhi = computeConfig_.initialPhi.scaled(
            downscaledImage_.size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }

    emit clearContourRequested();

#ifdef FLUVEL_DEBUG
    qDebug() << __FILE__ << ":" << __LINE__ << __func__
             << "phi:" << image_debug::describeImage(config.initialPhi);
#endif

    activeContourWorker_.initialize(downscaledImage_, config);
}

void ImageController::onProcessedImageReady(const QImage& processed)
{
    processedImage_ = processed;

    refreshView();
}

void ImageController::onImageSettingsChanged(const ImageSessionSettings& session)
{
    computeConfig_ = session.compute;

    reinitializeWorker();
}

void ImageController::onImageDisplaySettingsChanged(const DisplayConfig& display)
{
    bool needs_refresh = (displayConfig_.mode != display.mode);

    displayConfig_ = display;

    if (needs_refresh)
        refreshView();
}

void ImageController::refreshView()
{
    if (inputImage_.isNull() || processedImage_.isNull())
        return;

    QImage img;

    if (displayConfig_.mode == ImageDisplayMode::Source)
        img = inputImage_;
    else if (displayConfig_.mode == ImageDisplayMode::Preprocessed)
        img = processedImage_;

    emit displayedImageReady(img);
}

void ImageController::onContourUpdated(const fluvel_ip::ExportedContour& outerContour,
                                       const fluvel_ip::ExportedContour& innerContour)
{
    auto q_l_out = convertToQVector(outerContour);
    auto q_l_in = convertToQVector(innerContour);

    emit contourUpdated(q_l_out, q_l_in);
}

void ImageController::restart()
{
    activeContourWorker_.restart();
}

void ImageController::togglePause()
{
    activeContourWorker_.togglePause();
}

void ImageController::step()
{
    activeContourWorker_.step();
}

void ImageController::converge()
{
    activeContourWorker_.converge();
}

void ImageController::onStateChanged(fluvel_app::WorkerState state)
{
    emit stateChanged(state);
}

void ImageController::onDiagnosticsUpdated(const fluvel_ip::ContourDiagnostics& diag)
{
    auto formatChannels = [](const fluvel_ip::ChannelVector& v)
    {
        const auto& values = v.values();

        if (values.empty())
            return QString("n/a");

        if (values.size() == 1)
            return QString::number(values[0]);

        QString s("(");
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            s += QString::number(values[i]);
            if (i + 1 < values.size())
                s += ", ";
        }
        s += ")";
        return s;
    };

    QString s;

    s += QString("Step: %1\n").arg(diag.stepCount);
    s += QString("State: %1\n").arg(fluvel_ip::toString(diag.state));

    if (diag.stoppingStatus != fluvel_ip::StoppingStatus::None)
        s += QString("Reason: %1\n").arg(fluvel_ip::toString(diag.stoppingStatus));
    else
        s += QString("\n");

    if (!diag.meanInside.values().empty())
    {
        s += QString("Mean in: %1\n").arg(formatChannels(diag.meanInside));

        s += QString("Mean out: %1\n").arg(formatChannels(diag.meanOutside));
    }

    s += QString("Hausdorff q: %1 %\n").arg(diag.hausdorffQuantile, 0, 'g', 3);

    s += QString("Centroid dist: %1 %\n").arg(diag.relativeCentroidDistance, 0, 'g', 3);

    s += QString("Elapsed: %1 s\n").arg(diag.elapsedSec, 0, 'f', 2);

    s += QString("Contour: %1 pts").arg(diag.contourSize);

    emit textDiagnosticsUpdated(s);
}

} // namespace fluvel_app
