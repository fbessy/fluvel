#ifndef IMAGE_VIEW_HPP
#define IMAGE_VIEW_HPP

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QElapsedTimer>
#include <QTimer>
#include <QImage>

#include "contour_point_item.hpp"

namespace ofeli_app {

class ImageView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageView(QWidget* parent = nullptr);

    // Affichage image (thread-safe via event loop)
    void displayImage(const QImage& img);

    void clearOverlays();
    void updateSceneRectForImage();

    // Throttling : fps max (0 = désactivé)
    void setMaxDisplayFps(double fps);

    QImage currentImage() const;

public slots:
    void displayContour(const QVector<QPoint>& out,
                        const QVector<QPoint>& in);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void flushPendingFrame();

private:
    void updatePixmap(const QImage& img);

    void applyAutoView();
    void updateDragMode();
    double getCurrentZoom() const;

    QGraphicsScene*        scene = nullptr;
    QGraphicsPixmapItem*  pixmapItem = nullptr;

    bool autoViewEnabled = true;

    // --- Zoom / Pan ---
    const double minZoom = 0.1;
    const double maxZoom = 20.0;

    // --- Fullscreen ---
    bool isFullScreenMode = false;
    QRect normalGeometry;
    Qt::WindowFlags normalWindowFlags;

    // --- Throttling ---
    QImage pendingFrame;
    bool hasPendingFrame = false;

    QElapsedTimer displayTimer;
    int minDisplayIntervalMs = 0;

    QTimer* throttleTimer = nullptr;

    QImage lastDisplayedImage;

    QVector<QPoint> contourOut_;
    QVector<QPoint> contourIn_;

    ContourPointsItem* contourOutItem = nullptr;
    ContourPointsItem* contourInItem  = nullptr;

};

} // namespace ofeli_app

#endif // IMAGE_VIEW_BASE_H
