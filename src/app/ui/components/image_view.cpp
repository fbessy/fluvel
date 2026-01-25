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

    //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

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
        pixmapItem->setTransformationMode(Qt::FastTransformation);
        pixmapItem->setZValue(0);
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
        //updateDragMode();
    }
}

void ImageView::applyAutoView()
{
    if (!pixmapItem)
        return;

    //resetTransform();
    //fitInView(pixmapItem, Qt::KeepAspectRatio);
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

void ImageView::enableAutoView(bool enable)
{
    autoViewEnabled = enable;
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

    //updateDragMode();

#ifdef OFELI_DEBUG
    qDebug()
        << "mouseDelta =" << event->angleDelta().x() << event->angleDelta().y()
        << "zoom =" << currentZoom
        << " new zoom =" << newZoom
        << "delta =" << delta.x() << delta.y();
#endif
}

/*void ImageView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton) {
        autoViewEnabled = true;

        applyAutoView();
        updateDragMode();

        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}*/

/*void ImageView::mouseDoubleClickEvent(QMouseEvent* event)
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
}*/

void ImageView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    if ( autoFitEnabled ) {
        applyAutoFit();
    }

    //updateDragMode();
}

QImage ImageView::currentImage() const
{
    return lastDisplayedImage;
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

} // namespace ofeli_app
