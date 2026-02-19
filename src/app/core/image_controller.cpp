#include "image_controller.hpp"
#include "application_settings.hpp"

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
    bool isOk = false;

    if ( !inputImage_.isNull() )
    {
        if (    inputImage_.isGrayscale()
            && inputImage_.format() != QImage::Format_Grayscale8 )
        {
            inputImage_ = inputImage_.convertToFormat( QImage::Format_Grayscale8 );
        }
        else if ( inputImage_.format() != QImage::Format_RGB32 )
        {
            inputImage_ = inputImage_.convertToFormat( QImage::Format_RGB32 );
        }
    }

    if (    !inputImage_.isNull()
        &&
        (    inputImage_.format() == QImage::Format_Grayscale8
         || inputImage_.format() == QImage::Format_RGB32 ) )
    {
        isOk = true;
    }

    if ( isOk )
    {
        bool isResize = false;

        if ( !AppSettings::instance().imgConfig.compute.initialPhi.isNull() )
        {
            if ( AppSettings::instance().imgConfig.compute.initialPhi.width()  != inputImage_.width() ||
                 AppSettings::instance().imgConfig.compute.initialPhi.height() != inputImage_.height() )
            {
                AppSettings::instance().resize_initial_phi( inputImage_.width(),
                                                            inputImage_.height() );

                isResize = true;
            }
        }

        if ( isResize )
        {
            emit imageReadyWithResize(inputImage_);
        }
        else
        {
            emit imageReadyWithoutResize(inputImage_);
        }

        if ( AppSettings::instance().imgConfig.compute.initialPhi.width()  == inputImage_.width() &&
             AppSettings::instance().imgConfig.compute.initialPhi.height() == inputImage_.height() )
        {
            emit clearOverlaysRequested();

            acWorker.initializeFromInput(inputImage_,
                                         AppSettings::instance().imgConfig);
        }
    }
}

void ImageController::onProcessedImageReady(const QImage& processed)
{
    processedImage_ = processed;

    refreshView();
}

void ImageController::onImgSettingsChanged(const ImageSessionSettings& config)
{
    config_ = config;

    acWorker.setAlgoConfig( config_ );
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
    QVector<QPoint> q_l_out;
    QVector<QPoint> q_l_in;

    q_l_out.reserve(l_out.size());
    q_l_in.reserve(l_in.size());

    for (const auto& p : l_out)
        q_l_out.emplace_back( p.x, p.y );

    for (const auto& p : l_in)
        q_l_in.emplace_back( p.x, p.y );

    emit contourUpdated(q_l_out, q_l_in);
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
