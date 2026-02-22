#include "image_controller.hpp"
#include "application_settings.hpp"
#include "contour_adapters.hpp"

namespace ofeli_app
{

ImageController::ImageController(QObject* parent):
    QObject(parent)
{
    onImgSettingsChanged( AppSettings::instance().imgConfig );
    onImgDisplaySettingsChanged( AppSettings::instance().imgConfig.display );

    connect(&AppSettings::instance(),
            &ApplicationSettings::imgSettingsChanged,
            this,
            &ImageController::onImgSettingsChanged);

    connect(&AppSettings::instance(),
            &ApplicationSettings::imgDisplaySettingsChanged,
            this,
            &ImageController::onImgDisplaySettingsChanged);

    connect(&acWorker,  &ActiveContourWorker::processedImageReady,
            this,       &ImageController::onProcessedImageReady);

    connect(&acWorker,  &ActiveContourWorker::contourUpdated,
            this,       &ImageController::onContourUpdated,
            Qt::QueuedConnection);

    connect(&acWorker,  &ActiveContourWorker::stateChanged,
            this,       &ImageController::onStateChanged);
}

void ImageController::loadImage(const QString& path)
{
    inputImage_ = QImage(path);

    if ( inputImage_.isNull() )
        return;

    if ( inputImage_.isGrayscale() )
        inputImage_ = inputImage_.convertToFormat( QImage::Format_Grayscale8 );
    else
        inputImage_ = inputImage_.convertToFormat( QImage::Format_RGB32 );

    emit inputImageReady(inputImage_);

    reinitializeWorker();
}

void ImageController::downscaleImage()
{
    if ( inputImage_.isNull() )
        return;

    if (    inputImage_.format() != QImage::Format_Grayscale8
        && inputImage_.format() != QImage::Format_RGB32 )
        return;

    if ( computeConfig_.downscale.hasDownscale )
    {
        const int df = computeConfig_.downscale.downscaleFactor;

        assert( df == 2 || df == 4 );

        downscaledImage_ = inputImage_.scaled(inputImage_.width() / df,
                                              inputImage_.height() / df,
                                              Qt::IgnoreAspectRatio,
                                              Qt::FastTransformation);
    }
    else
    {
        downscaledImage_ = inputImage_;
    }

    QImage& phi = computeConfig_.initialPhi;

    phi = phi.scaled(downscaledImage_.width(),
                     downscaledImage_.height(),
                     Qt::IgnoreAspectRatio,
                     Qt::FastTransformation);
}

void ImageController::reinitializeWorker()
{
    downscaleImage();

    emit clearOverlaysRequested();

    acWorker.initialize(downscaledImage_,
                        computeConfig_);
}

void ImageController::onProcessedImageReady(const QImage& processed)
{
    processedImage_ = processed;

    refreshView();
}

void ImageController::onImgSettingsChanged(const ImageSessionSettings& config)
{
    computeConfig_ = config.compute;

    reinitializeWorker();
}

void ImageController::onImgDisplaySettingsChanged(const DisplayConfig& display)
{
    bool needs_refresh = ( displayConfig_.input_displayed != display.input_displayed );

    displayConfig_ = display;

    if ( needs_refresh )
        refreshView();
}

void ImageController::refreshView()
{
    if ( inputImage_.isNull() || processedImage_.isNull() )
        return;

    QImage img;

    if ( displayConfig_.input_displayed )
        img = inputImage_;
    else
        img = processedImage_;


    emit displayedImageReady( img );
}

void ImageController::onContourUpdated(const ofeli_ip::ExportedContour& l_out,
                                       const ofeli_ip::ExportedContour& l_in)
{
    auto q_l_out = convertToQVector( l_out );
    auto q_l_in  = convertToQVector( l_in );

    emit contourUpdated( q_l_out, q_l_in );
}

void ImageController::restart()
{
    acWorker.restart();
}

void ImageController::togglePause()
{
    acWorker.togglePause();
}

void ImageController::step()
{
    acWorker.step();
}

void ImageController::converge()
{
    acWorker.converge();
}

void ImageController::onStateChanged(ofeli_app::WorkerState state)
{
    emit stateChanged( state );
}

}
