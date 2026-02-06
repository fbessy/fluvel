#include "image_controller.hpp"
#include "application_settings.hpp"

namespace ofeli_app {

ImageController::ImageController(QObject* parent):
    QObject(parent)
{
    connect(&AppSettings::instance(), &ApplicationSettings::imgDisplaySettingsChanged,
            this, &ImageController::setDisplayedImage);
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

        if ( !AppSettings::instance().imgSessSettings.initial_phi.isNull() )
        {
            if ( AppSettings::instance().imgSessSettings.initial_phi.width()  != inputImage_.width() ||
                 AppSettings::instance().imgSessSettings.initial_phi.height() != inputImage_.height() )
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

        emit inputImageReady(inputImage_);
    }
}

void ImageController::onProcessedImageReady(const QImage& processed)
{
    processedImage_ = processed;

    setDisplayedImage();
}

void ImageController::setDisplayedImage()
{
    if ( AppSettings::instance().imgSessSettings.img_disp_conf.input_displayed)
        displayedImage_ = inputImage_;
    else
        displayedImage_ = processedImage_;


    emit displayedImageReady(displayedImage_);
}

}
