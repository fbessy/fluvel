#ifndef IMAGE_VIEW_HPP
#define IMAGE_VIEW_HPP

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QElapsedTimer>
#include <QTimer>
#include <QImage>

#include "contour_point_item.hpp"
#include "image_view_listener.hpp"
#include "common_settings.hpp"
#include "application_settings.hpp"


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

    // Throttling : fps max (0 = désactivé)
    void setMaxDisplayFps(double fps);

    const QImage& image() const;
    QImage renderToImage() const;

    void setInteraction(ImageViewInteraction* interaction);

    double currentZoom() const;
    void scaleView(double sx, double sy);
    void translateView(double dx, double dy);

    void toggleFullscreen();
    void applyAutoFit();
    void userInteracted();

    QPoint imageCoordinatesFromView(const QPoint& viewPos) const;
    QRgb pixelColorAt(const QPoint& imagePos) const;

    void setListener(ImageViewListener* listener);
    void onColorPicked(const QColor& color,
                       const QPoint& imagePos);

    bool hasImage() const;
    bool isPanRelevant() const;
    QGraphicsScene* graphicsScene() const;

    bool isGrayscale() const;

public slots:
    void setImage(const QImage& img);

    void setContour(const QVector<QPoint>& l_out,
                    const QVector<QPoint>& l_in);

    void setImageAndContour(const QImage& image,
                            const QVector<QPoint>& l_out,
                            const QVector<QPoint>& l_in,
                            qint64 receiveTs);

    void applyDisplayConfig(const DisplayConfig& display);
    void applyDownscaleConfig(const DownscaleConfig& downscale);
    void updateFlip();
    void clearOverlays();

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent*) override;

    void resizeEvent(QResizeEvent* event) override;

private slots:
    void flushPendingFrame();

private:
    void updatePixmap(const QImage& img);
    void upscaleItems();
    double getCurrentZoom() const;
    void updateDisplayWithConfig();

    QGraphicsScene*        scene = nullptr;
    QGraphicsPixmapItem*  pixmapItem = nullptr;

    bool autoFitEnabled = true;

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

    ContourPointsItem* l_out_ = nullptr;
    ContourPointsItem* l_in_  = nullptr;

    ImageViewInteraction* m_interaction = nullptr;
    ImageViewListener*        listener_ = nullptr;

    DisplayConfig displayConfig_;
    DownscaleConfig downscaleConfig_;

    qint64 lastReceiveTs_;

    QGraphicsItemGroup* contentRoot_;

signals:
    void imageClicked(int x, int y);
    void frameDisplayed(qint64 receiveTs,
                        qint64 displayTs);
};

} // namespace ofeli_app

#endif // IMAGE_VIEW_BASE_HPP
