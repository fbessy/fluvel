#ifndef IMAGE_VIEW_HPP
#define IMAGE_VIEW_HPP

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QElapsedTimer>
#include <QTimer>
#include <QImage>

#include "contour_point_item.hpp"


class QWheelEvent;
class QMouseEvent;
class QResizeEvent;


namespace ofeli_app
{

class ImageViewInteraction;

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
    void setInteraction(ImageViewInteraction* interaction);

    void applyAutoView();
    void updateDragMode();
    double currentZoom() const;
    void scaleView(double sx, double sy);
    void translateView(double dx, double dy);
    void enableAutoView(bool enable);
    void toggleFullScreen();

public slots:
    void displayContour(const QVector<QPoint>& out,
                        const QVector<QPoint>& in);

protected:
    void wheelEvent(::QWheelEvent* event) override;
    void mousePressEvent(::QMouseEvent* event) override;
    void mouseDoubleClickEvent(::QMouseEvent* event) override;

    void resizeEvent(QResizeEvent* event) override;

private slots:
    void flushPendingFrame();

private:
    void updatePixmap(const QImage& img);
    double getCurrentZoom() const;

    QGraphicsScene*        scene = nullptr;
    QGraphicsPixmapItem*  pixmapItem = nullptr;

    bool autoViewEnabled = true;

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

    ImageViewInteraction* m_interaction = nullptr;
};

} // namespace ofeli_app

#endif // IMAGE_VIEW_BASE_H
