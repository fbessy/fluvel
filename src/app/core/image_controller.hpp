#ifndef IMAGE_CONTROLLER_HPP
#define IMAGE_CONTROLLER_HPP

#include <QImage>
#include <QObject>

namespace ofeli_gui {

class ImageController : public QObject
{
Q_OBJECT

public:
    ImageController(QWidget* parent);
    void applyFilters();

public slots:
    void loadImage(const QString& path);

signals:
    void imageReady(const QImage& image);
    void contourReady(const QImage& image);

private:
    QImage img;
    QImage filtered;
};

}

#endif // IMAGE_CONTROLLER_HPP
