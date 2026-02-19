#ifndef IMAGE_CONTROLLER_HPP
#define IMAGE_CONTROLLER_HPP

#include "active_contour_worker.hpp"

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
    void onContourUpdated(const ofeli_ip::ExportedContour& l_out,
                          const ofeli_ip::ExportedContour& l_in);
    void onStateChanged(ofeli_app::WorkerState state);

    void restart();
    void togglePause();
    void step();
    void converge();

signals:
    void inputImageReady(const QImage& image);
    void imageReadyWithResize(const QImage& image);
    void imageReadyWithoutResize(const QImage& image);
    void displayedImageReady(const QImage& image);

    void contourUpdated(const QVector<QPoint>& l_out,
                        const QVector<QPoint>& l_in);
    void stateChanged(ofeli_app::WorkerState state);

    void clearOverlaysRequested();

private:

    void onImgSettingsChanged(const ImageSessionSettings& conf);
    void onImgDisplaySettingsChanged(const DisplayConfig& display);
    void refreshView();

    QImage inputImage_;
    QImage processedImage_;
    DisplayConfig displayConfig_;

    ActiveContourWorker acWorker;
    ImageSessionSettings config_;
};

}

#endif // IMAGE_CONTROLLER_HPP
