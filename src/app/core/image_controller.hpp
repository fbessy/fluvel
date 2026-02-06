#ifndef IMAGE_CONTROLLER_HPP
#define IMAGE_CONTROLLER_HPP

#include <QImage>
#include <QObject>

namespace ofeli_app {

class ImageController : public QObject
{
Q_OBJECT

public:
    ImageController(QObject* parent);

public slots:
    void loadImage(const QString& path);
    void onProcessedImageReady(const QImage& processed);

signals:
    void inputImageReady(const QImage& image);
    void imageReadyWithResize(const QImage& image);
    void imageReadyWithoutResize(const QImage& image);
    void displayedImageReady(const QImage& image);

private:
    void setDisplayedImage();

    QImage inputImage_;
    QImage processedImage_;
    QImage displayedImage_;
};

}

#endif // IMAGE_CONTROLLER_HPP
