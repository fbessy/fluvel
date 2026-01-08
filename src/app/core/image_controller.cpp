#include "image_controller.hpp"

namespace ofeli_gui {

ImageController::ImageController(QWidget* parent)
{
    connect(this, &ImageController::imageReady,
            this, &ImageController::applyFilters);
}

void ImageController::loadImage(const QString& path)
{
    img = QImage(path);

    if (img.isNull())
        return;

    // --- Cas 1 : déjà le format idéal ---
    if (img.format() == QImage::Format_Grayscale8 ||
        img.format() == QImage::Format_RGB32)
    {
        emit imageReady(img);
        return;
    }

    // --- Cas 2 : image détectée comme grayscale ---
    // (y compris RGB32 avec R=G=B)
    if (img.isGrayscale()) {

        // On force une représentation HONNÊTE
        if (img.format() != QImage::Format_Grayscale8) {
            QImage gray(img.width(), img.height(),
                        QImage::Format_Grayscale8);

            for (int y = 0; y < img.height(); ++y) {
                const QRgb* src =
                    reinterpret_cast<const QRgb*>(img.constScanLine(y));
                uchar* dst = gray.scanLine(y);

                for (int x = 0; x < img.width(); ++x)
                    dst[x] = qRed(src[x]); // R=G=B garanti
            }
            emit imageReady(gray);
            return;
        }

        emit imageReady(img);
        return;
    }

    // --- Cas 3 : vraie couleur ---
    // On normalise explicitement en BGR32
    if (img.format() != QImage::Format_RGB32)
        img = img.convertToFormat(QImage::Format_RGB32);

    emit imageReady(img);
}

void ImageController::applyFilters()
{
    filtered = img;
    emit contourReady(filtered);
}

}
