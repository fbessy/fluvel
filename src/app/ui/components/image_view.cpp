#include "image_view.hpp"
#include "image_view_interaction.hpp"
#include "application_settings.hpp"

#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QThread>

namespace ofeli_app
{

ImageView::ImageView(QWidget* parent)
    : QGraphicsView(parent),
      scene(new QGraphicsScene(this))
{
    setScene(scene);

    setRenderHint(QPainter::SmoothPixmapTransform, false);
    setAlignment(Qt::AlignCenter);

    setTransformationAnchor(QGraphicsView::NoAnchor);
    setResizeAnchor(QGraphicsView::NoAnchor);
    setDragMode(QGraphicsView::NoDrag);

    //AppSettings.instance().color_in

    contourOutItem = new ContourPointsItem;
    contourOutItem->setColor( Qt::red );
    contourOutItem->setZValue(100);
    scene->addItem(contourOutItem);

    contourInItem = new ContourPointsItem;
    contourInItem->setColor( Qt::green );
    contourInItem->setZValue(100);
    scene->addItem(contourInItem);

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
    if (minDisplayIntervalMs == 0)
    {
        updatePixmap(pendingFrame);
        hasPendingFrame = false;
        displayTimer.restart();
        return;
    }

    const qint64 elapsed = displayTimer.elapsed();

    if (elapsed >= minDisplayIntervalMs)
    {
        updatePixmap(pendingFrame);
        hasPendingFrame = false;
        displayTimer.restart();
    }
    else
    {
        if (!throttleTimer->isActive())
        {
            throttleTimer->start(minDisplayIntervalMs - elapsed);
        }
    }
}

void ImageView::displayContour(const QVector<QPoint>& out,
                               const QVector<QPoint>& in)
{
    if (!contourOutItem || !contourInItem)
        return;


    const qreal factor = qreal( AppSettings::instance().downscale_factor );

    contourOutItem->setPoints(out);
    contourInItem->setPoints(in);

    contourOutItem->setScale( factor );
    contourInItem->setScale( factor );
}

void ImageView::clearOverlays()
{
    if ( contourOutItem )
    {
        contourOutItem->clearPoints();
    }

    if ( contourInItem )
    {
        contourInItem->clearPoints();
    }
}

void ImageView::updateSceneRectForImage()
{
    //viewport()->repaint();
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
    const bool newImage = !pixmapItem ||
                          pixmapItem->pixmap().size() != img.size();

    lastDisplayedImage = img;

    if (!pixmapItem)
    {
        pixmapItem = scene->addPixmap(QPixmap::fromImage(img));
    }
    else
    {
        pixmapItem->setTransformationMode(Qt::FastTransformation);
        pixmapItem->setZValue(0);
        pixmapItem->setPixmap(QPixmap::fromImage(img));
    }
    scene->setSceneRect(pixmapItem->boundingRect());

    if (newImage)
    {
        applyAutoFit();
    }
}

double ImageView::currentZoom() const
{
    return getCurrentZoom();
}

void ImageView::scaleView(double sx, double sy)
{
    scale(sx, sy);
}

void ImageView::translateView(double dx, double dy)
{
    translate(dx, dy);
}

double ImageView::getCurrentZoom() const
{
    return transform().m11(); // scale X
}

void ImageView::toggleFullscreen()
{
    if (!isFullScreenMode)
    {
        normalGeometry = window()->geometry();
        normalWindowFlags = window()->windowFlags();
        window()->setWindowFlags(Qt::Window);
        window()->showFullScreen();

        isFullScreenMode = true;
    }
    else
    {
        window()->setWindowFlags(normalWindowFlags);
        window()->showNormal();
        window()->setGeometry(normalGeometry);
        isFullScreenMode = false;
    }
}

void ImageView::applyAutoFit()
{
    if (!pixmapItem)
        return;

    autoFitEnabled = true;
    resetTransform();
    fitInView(pixmapItem, Qt::KeepAspectRatio);
}

void ImageView::userInteracted()
{
    autoFitEnabled = false;
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

    autoFitEnabled = false;

#ifdef OFELI_DEBUG
    qDebug()
        << "mouseDelta =" << event->angleDelta().x() << event->angleDelta().y()
        << "zoom =" << currentZoom
        << " new zoom =" << newZoom
        << "delta =" << delta.x() << delta.y();
#endif
}

void ImageView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    if (autoFitEnabled)
        applyAutoFit();
}

void ImageView::setInteraction(ImageViewInteraction* interaction)
{
    m_interaction = interaction;
}

/*void ImageView::wheelEvent(QWheelEvent* event)
{
    if (m_interaction)
    {
        m_interaction->wheel(*this, event);
        event->accept();
        return;
    }

    QGraphicsView::wheelEvent(event);
}*/

void ImageView::mousePressEvent(QMouseEvent* event)
{
    if ( m_interaction )
        m_interaction->mousePress(*this, event);

    if ( !event->isAccepted() )
        QGraphicsView::mousePressEvent(event);
}

void ImageView::mouseMoveEvent(QMouseEvent* event)
{
    if ( m_interaction )
        m_interaction->mouseMove(*this, event);

    if ( !event->isAccepted() )
        QGraphicsView::mouseMoveEvent(event);
}

void ImageView::mouseReleaseEvent(QMouseEvent* event)
{
    if ( m_interaction )
        m_interaction->mouseRelease(*this, event);

    if ( !event->isAccepted() )
        QGraphicsView::mouseReleaseEvent(event);
}

void ImageView::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (m_interaction)
        m_interaction->mouseDoubleClick(*this, event);

    if ( !event->isAccepted() )
        QGraphicsView::mouseDoubleClickEvent(event);
}

QPoint ImageView::imageCoordinatesFromView(const QPoint& viewPos) const
{
    if (!pixmapItem)
        return QPoint(-1, -1);

    QPointF scenePos = mapToScene(viewPos);
    QPointF itemPos  = pixmapItem->mapFromScene(scenePos);

    QPoint p = itemPos.toPoint();

    if (!pixmapItem->boundingRect().contains(p))
        return QPoint(-1, -1);

    return p;
}

QColor ImageView::pixelColorAt(const QPoint& imagePos) const
{
    if (!lastDisplayedImage.valid(imagePos))
        return QColor();

    return QColor::fromRgb(lastDisplayedImage.pixel(imagePos));
}

QImage ImageView::currentImage() const
{
    return lastDisplayedImage;
}

QImage ImageView::renderToImage() const
{
    if (!scene || scene->sceneRect().isEmpty())
        return QImage();

    QRectF rect = scene->sceneRect();

    QImage img(rect.size().toSize(),
               QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    scene->render(&painter,
                  QRectF(img.rect()),
                  rect,
                  Qt::IgnoreAspectRatio);

    return img;
}

} // namespace ofeli_app
