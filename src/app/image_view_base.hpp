#ifndef IMAGE_VIEW_BASE_HPP
#define IMAGE_VIEW_BASE_HPP

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QElapsedTimer>
#include <QTimer>
#include <QImage>

namespace ofeli_gui {

class ImageViewBase : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageViewBase(QWidget* parent = nullptr);

    // Affichage image (thread-safe via event loop)
    void displayImage(const QImage& img);

    // Throttling : fps max (0 = désactivé)
    void setMaxDisplayFps(double fps);

    QImage currentImage() const;

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void flushPendingFrame();

private:
    void updatePixmap(const QImage& img);
    void updateDragMode();

private:
    QGraphicsScene*        scene = nullptr;
    QGraphicsPixmapItem*  pixmapItem = nullptr;

    bool autoViewEnabled = true;

    // --- Zoom / Pan ---
    double currentZoom = 1.0;
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
};

} // namespace ofeli_gui

#endif // IMAGE_VIEW_BASE_H
