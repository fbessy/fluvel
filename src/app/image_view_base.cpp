#include "image_view_base.hpp"

#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QThread>

namespace ofeli_gui {

ImageViewBase::ImageViewBase(QWidget* parent)
    : QGraphicsView(parent),
      scene(new QGraphicsScene(this))
{
    setScene(scene);

    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setAlignment(Qt::AlignCenter);

    setTransformationAnchor(QGraphicsView::NoAnchor);
    setResizeAnchor(QGraphicsView::NoAnchor);
    setDragMode(QGraphicsView::NoDrag);

    displayTimer.start();

    throttleTimer = new QTimer(this);
    throttleTimer->setSingleShot(true);

    connect(throttleTimer, &QTimer::timeout,
            this, &ImageViewBase::flushPendingFrame);
}

// ------------------------------------------------------------
// Public API
// ------------------------------------------------------------
void ImageViewBase::setMaxDisplayFps(double fps)
{
    if (fps <= 0.0) {
        minDisplayIntervalMs = 0;
        return;
    }

    minDisplayIntervalMs = static_cast<int>(1000.0 / fps);
}

void ImageViewBase::displayImage(const QImage& img)
{
    // Toujours garder la dernière image
    pendingFrame = img;
    hasPendingFrame = true;

    // Pas de throttling
    if (minDisplayIntervalMs == 0) {
        updatePixmap(pendingFrame);
        hasPendingFrame = false;
        displayTimer.restart();
        return;
    }

    const qint64 elapsed = displayTimer.elapsed();

    if (elapsed >= minDisplayIntervalMs) {
        updatePixmap(pendingFrame);
        hasPendingFrame = false;
        displayTimer.restart();
    }
    else {
        if (!throttleTimer->isActive()) {
            throttleTimer->start(minDisplayIntervalMs - elapsed);
        }
    }
}

// ------------------------------------------------------------
// Internal rendering
// ------------------------------------------------------------
void ImageViewBase::flushPendingFrame()
{
    if (!hasPendingFrame)
        return;

    updatePixmap(pendingFrame);
    hasPendingFrame = false;
    displayTimer.restart();
}

void ImageViewBase::updatePixmap(const QImage& img)
{
    lastDisplayedImage = img;

    const bool firstFrame = (pixmapItem == nullptr);

    if (!pixmapItem) {
        pixmapItem = scene->addPixmap(QPixmap::fromImage(img));
    } else {
        pixmapItem->setPixmap(QPixmap::fromImage(img));
    }

    scene->setSceneRect(pixmapItem->boundingRect());

    if (firstFrame || autoViewEnabled) {
        resetTransform();
        currentZoom = 1.0;
        fitInView(pixmapItem, Qt::KeepAspectRatio);
        updateDragMode();
    }
}

void ImageViewBase::updateDragMode()
{
    if (autoViewEnabled)
        setDragMode(QGraphicsView::NoDrag);
    else
        setDragMode(QGraphicsView::ScrollHandDrag);
}

// ------------------------------------------------------------
// Interaction
// ------------------------------------------------------------
void ImageViewBase::wheelEvent(QWheelEvent* event)
{
    constexpr double zoomFactor = 1.15;

    QPointF scenePosBefore = mapToScene(event->position().toPoint());

    double factor = (event->angleDelta().y() > 0)
                        ? zoomFactor
                        : 1.0 / zoomFactor;

    double newZoom = currentZoom * factor;
    if (newZoom < minZoom || newZoom > maxZoom)
        return;

    currentZoom = newZoom;
    scale(factor, factor);

    QPointF scenePosAfter = mapToScene(event->position().toPoint());
    QPointF delta = scenePosAfter - scenePosBefore;
    translate(delta.x(), delta.y());

    autoViewEnabled = false;

    if (currentZoom > 1.01)
        setDragMode(QGraphicsView::ScrollHandDrag);
    else
        setDragMode(QGraphicsView::NoDrag);
}

void ImageViewBase::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton) {
        autoViewEnabled = true;
        resetTransform();
        currentZoom = 1.0;

        if (pixmapItem)
            fitInView(pixmapItem, Qt::KeepAspectRatio);

        updateDragMode();

        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void ImageViewBase::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    if (!isFullScreenMode) {
        normalGeometry = geometry();
        normalWindowFlags = windowFlags();

        setWindowFlags(Qt::Window);
        showFullScreen();
        isFullScreenMode = true;
    }
    else {
        setWindowFlags(normalWindowFlags);
        showNormal();

        QTimer::singleShot(0, this, [this]() {
            setGeometry(normalGeometry);
        });

        isFullScreenMode = false;
    }
}

void ImageViewBase::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    if (pixmapItem && autoViewEnabled) {
        resetTransform();
        currentZoom = 1.0;
        fitInView(pixmapItem, Qt::KeepAspectRatio);
        updateDragMode();
    }
}

QImage ImageViewBase::currentImage() const
{
    return lastDisplayedImage;
}

} // namespace ofeli_gui
