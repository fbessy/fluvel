#include "image_controller.hpp"
#include "application_settings.hpp"

namespace ofeli_app {

ImageController::ImageController(QObject* parent):
    QObject(parent)
{
    connect(this, &ImageController::imageReady,
            this, &ImageController::applyFilters);
}

void ImageController::loadImage(const QString& path)
{
    img = QImage(path);
    bool isOk = false;

    if ( !img.isNull() )
    {
        if ( img.isGrayscale() && img.format() != QImage::Format_Grayscale8 )
        {
            img = img.convertToFormat( QImage::Format_Grayscale8 );
        }
        else if ( img.format() != QImage::Format_RGB32 )
        {
            img = img.convertToFormat( QImage::Format_RGB32 );
        }
    }

    if ( !img.isNull() &&
         ( img.format() == QImage::Format_Grayscale8 || img.format() == QImage::Format_RGB32 ) )
    {
        isOk = true;
    }

    if ( isOk )
    {
        bool isResize = false;

        if ( !AppSettings::instance().imgSessSettings.initial_phi.isNull() )
        {
            if ( AppSettings::instance().imgSessSettings.initial_phi.width()  != img.width() ||
                 AppSettings::instance().imgSessSettings.initial_phi.height() != img.height() )
            {
                AppSettings::instance().resize_initial_phi( img.width(),
                                                            img.height() );

                isResize = true;
            }
        }

        if ( isResize )
        {
            emit imageReadyWithResize(img);
        }
        else
        {
            emit imageReadyWithoutResize(img);
        }

        emit imageReady(img);
    }
}

void ImageController::applyFilters()
{
    filtered = img;
    emit contourReady(filtered);
}

}
