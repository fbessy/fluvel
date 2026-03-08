// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "image_controller.hpp"

#include "application_settings.hpp"
#include "contour_adapters.hpp"

#include <QImageReader>

namespace fluvel_app
{

ImageController::ImageController(QObject* parent)
    : QObject(parent)
{
    onImgSettingsChanged(AppSettings::instance().imgConfig);
    onImgDisplaySettingsChanged(AppSettings::instance().imgConfig.display);

    connect(&AppSettings::instance(), &ApplicationSettings::imgSettingsChanged, this,
            &ImageController::onImgSettingsChanged);

    connect(&AppSettings::instance(), &ApplicationSettings::imgDisplaySettingsChanged, this,
            &ImageController::onImgDisplaySettingsChanged);

    connect(&acWorker_, &ActiveContourWorker::processedImageReady, this,
            &ImageController::onProcessedImageReady);

    connect(&acWorker_, &ActiveContourWorker::contourUpdated, this,
            &ImageController::onContourUpdated, Qt::QueuedConnection);

    connect(&acWorker_, &ActiveContourWorker::stateChanged, this, &ImageController::onStateChanged);

    connect(&acWorker_, &ActiveContourWorker::diagnosticsUpdated, this,
            &ImageController::onDiagnosticsUpdated);
}

void ImageController::loadImage(const QString& path)
{
    if (path.isEmpty())
        return;

    if (!isSupportedImage(path))
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

    if (originalInitialPhi_.isNull())
        return;

    computeConfig_.initialPhi =
        originalInitialPhi_.scaled(downscaledImage_.width(), downscaledImage_.height(),
                                   Qt::IgnoreAspectRatio, Qt::FastTransformation);

    if (computeConfig_.initialPhi.isNull())
        return;

    emit clearOverlaysRequested();

    acWorker_.initialize(downscaledImage_, computeConfig_);
}

void ImageController::onProcessedImageReady(const QImage& processed)
{
    processedImage_ = processed;

    refreshView();
}

void ImageController::onImgSettingsChanged(const ImageSessionSettings& config)
{
    computeConfig_ = config.compute;
    originalInitialPhi_ = computeConfig_.initialPhi;

    reinitializeWorker();
}

void ImageController::onImgDisplaySettingsChanged(const DisplayConfig& display)
{
    bool needs_refresh = (displayConfig_.image != display.image);

    displayConfig_ = display;

    if (needs_refresh)
        refreshView();
}

void ImageController::refreshView()
{
    if (inputImage_.isNull() || processedImage_.isNull())
        return;

    QImage img;

    if (displayConfig_.image == ImageBase::Source)
        img = inputImage_;
    else if (displayConfig_.image == ImageBase::Preprocessed)
        img = processedImage_;

    emit displayedImageReady(img);
}

void ImageController::onContourUpdated(const fluvel_ip::ExportedContour& l_out,
                                       const fluvel_ip::ExportedContour& l_in)
{
    auto q_l_out = convertToQVector(l_out);
    auto q_l_in = convertToQVector(l_in);

    emit contourUpdated(q_l_out, q_l_in);
}

void ImageController::restart()
{
    acWorker_.restart();
}

void ImageController::togglePause()
{
    acWorker_.togglePause();
}

void ImageController::step()
{
    acWorker_.step();
}

void ImageController::converge()
{
    acWorker_.converge();
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

    if (!diag.meanIn.values().empty())
    {
        s += QString("Mean in: %1\n").arg(formatChannels(diag.meanIn));

        s += QString("Mean out: %1\n").arg(formatChannels(diag.meanOut));
    }

    s += QString("Hausdorff q: %1 %\n").arg(diag.hausdorffQuantile, 0, 'g', 3);

    s += QString("Centroid dist: %1 %\n").arg(diag.relativeCentroidDistance, 0, 'g', 3);

    s += QString("Elapsed: %1 s\n").arg(diag.elapsedSec, 0, 'f', 2);

    s += QString("Contour: %1 pts").arg(diag.contourSize);

    emit textDiagnosticsUpdated(s);
}

bool isSupportedImage(const QString& path)
{
    QImageReader reader(path);
    return reader.canRead();
}

} // namespace fluvel_app
