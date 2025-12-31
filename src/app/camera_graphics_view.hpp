#ifndef CAMERA_GRAPHICS_VIEW_HPP
#define CAMERA_GRAPHICS_VIEW_HPP

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QImage>
#include <QMutex>
#include <QRect>
#include <QBoxLayout>

namespace ofeli_gui {

class CameraGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CameraGraphicsView(QWidget* parent = nullptr);

    void displayImage(const QImage& img);

    void setInputFps(float fps);
    void setProcessingFps(float fps);
    void setDisplayFps(float fps);
    void setDropRate(float dr);
    void setAvgLatencyMs(float lat);
    void setMaxLatencyMs(float lat);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:

    QColor contrastColorFast(const QImage &img, const QRect &area);

    void drawTextOverlay(QPainter &painter, const QImage &img,
                         const QString &text, const QPoint &pos,
                         int fontSize);

    QGraphicsScene* scene;
    QGraphicsPixmapItem* pixmapItem;
    bool isPainting;
    bool isFullScreenMode;
    QRect normalGeometry;
    Qt::WindowFlags normalWindowFlags;

    QMutex frameMutex;
    QImage lastFrame;

    float inputFps = 0.f;
    float processingFps = 0.f;
    float displayFps = 0.f;
    float dropRate = 0.f;
    float avgLatencyMs = 0.f;
    float maxLatencyMs = 0.f;

    double currentScale = 1.0;
    const double minScale = 0.1;
    const double maxScale = 10.0;

    QWidget* normalParent;
    QLayout* normalLayout;
    int normalIndex;

    double currentZoom = 1.0;
    const double minZoom = 0.1;
    const double maxZoom = 10.0;
};

} // namespace ofeli_gui

#endif // CAMERA_GRAPHICS_VIEW_HPP
