#include "image_view.hpp"

#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QThread>

namespace ofeli_gui {

ImageView::ImageView(QWidget* parent)
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
            this, &ImageView::flushPendingFrame);
}

// ------------------------------------------------------------
// Public API
// ------------------------------------------------------------
void ImageView::setMaxDisplayFps(double fps)
{
    if (fps <= 0.0) {
        minDisplayIntervalMs = 0;
        return;
    }

    minDisplayIntervalMs = static_cast<int>(1000.0 / fps);
}

void ImageView::displayImage(const QImage& img)
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
void ImageView::flushPendingFrame()
{
    if (!hasPendingFrame)
        return;

    updatePixmap(pendingFrame);
    hasPendingFrame = false;
    displayTimer.restart();
}

void ImageView::updatePixmap(const QImage& img)
{
    bool needUpdateDragMode = false;

    if( lastDisplayedImage.width()  != img.width() ||
        lastDisplayedImage.height() != img.height()   )
    {
        needUpdateDragMode = true;
    }

    lastDisplayedImage = img;

    const bool firstFrame = (pixmapItem == nullptr);

    if (!pixmapItem)
    {
        pixmapItem = scene->addPixmap(QPixmap::fromImage(img));
    }
    else
    {
        pixmapItem->setPixmap(QPixmap::fromImage(img));
    }

    scene->setSceneRect(pixmapItem->boundingRect());

    if (firstFrame || autoViewEnabled)
    {
        applyAutoView();
        needUpdateDragMode = true;
    }

    if( needUpdateDragMode )
    {
        updateDragMode();
    }
}

void ImageView::applyAutoView()
{
    if (!pixmapItem)
        return;

    resetTransform();
    fitInView(pixmapItem, Qt::KeepAspectRatio);
}

void ImageView::updateDragMode()
{
    if ( !scene )
        return;

    if ( autoViewEnabled )
    {
        setDragMode(QGraphicsView::NoDrag);
    }
    else
    {
        QRectF sceneRect = scene->sceneRect();
        QRectF viewRect  = mapToScene(viewport()->rect()).boundingRect();

        bool canPan =
            sceneRect.width()  > viewRect.width() ||
            sceneRect.height() > viewRect.height();

        setDragMode(canPan
                        ? QGraphicsView::ScrollHandDrag
                        : QGraphicsView::NoDrag);
    }
}

// ------------------------------------------------------------
// Interaction
// ------------------------------------------------------------
void ImageView::wheelEvent(QWheelEvent* event)
{
    constexpr double zoomFactor = 1.15;

    QPointF scenePosBefore = mapToScene(event->position().toPoint());

    double factor = (event->angleDelta().y() > 0)
                        ? zoomFactor
                        : 1.0 / zoomFactor;

    double currentZoom = getCurrentZoom();
    double newZoom = currentZoom * factor;

    if (newZoom < minZoom || newZoom > maxZoom)
        return;

    scale(factor, factor);

    QPointF scenePosAfter = mapToScene(event->position().toPoint());
    QPointF delta = scenePosAfter - scenePosBefore;
    translate(delta.x(), delta.y());

    autoViewEnabled = false;

    updateDragMode();

#ifdef OFELI_DEBUG
    qDebug()
        << "mouseDelta =" << event->angleDelta().x() << event->angleDelta().y()
        << "zoom =" << currentZoom
        << " new zoom =" << newZoom
        << "delta =" << delta.x() << delta.y();
#endif
}

void ImageView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton) {
        autoViewEnabled = true;

        applyAutoView();
        updateDragMode();

        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void ImageView::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    if (!isFullScreenMode)
    {
        normalGeometry = geometry();
        normalWindowFlags = windowFlags();

        setWindowFlags(Qt::Window);
        showFullScreen();
        isFullScreenMode = true;

        autoViewEnabled = true;
        applyAutoView();
    }
    else
    {
        setWindowFlags(normalWindowFlags);
        showNormal();

        QTimer::singleShot(0, this, [this]() {
            setGeometry(normalGeometry);
        });

        isFullScreenMode = false;
    }

    updateDragMode();
}

void ImageView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    if ( autoViewEnabled ) {
        applyAutoView();
    }

    updateDragMode();
}

double ImageView::getCurrentZoom() const
{
    return transform().m11(); // scale X
}

QImage ImageView::currentImage() const
{
    return lastDisplayedImage;
}

} // namespace ofeli_gui
