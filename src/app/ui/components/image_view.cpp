#include "image_view.hpp"
#include "image_view_interaction.hpp"
#include "application_settings.hpp"
#include "common_settings.hpp"
#include "color_adapters.hpp"
#include "frame_clock.hpp"

#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QThread>

namespace ofeli_app
{

ImageView::ImageView(QWidget* parent)
    : QGraphicsView(parent)
{
    setRenderHint(QPainter::SmoothPixmapTransform, false);
    setAlignment(Qt::AlignCenter);

    setTransformationAnchor(QGraphicsView::NoAnchor);
    setResizeAnchor(QGraphicsView::NoAnchor);
    setDragMode(QGraphicsView::NoDrag);

    scene = new QGraphicsScene(this);
    setScene(scene);

    contentRoot_ = new QGraphicsItemGroup;
    scene->addItem(contentRoot_);

    pixmapItem = new QGraphicsPixmapItem(contentRoot_);
    pixmapItem->setZValue(0);
    pixmapItem->setTransformationMode(Qt::FastTransformation);

    l_out_ = new ContourPointsItem(contentRoot_);
    l_out_->setZValue(100);

    l_in_ = new ContourPointsItem(contentRoot_);
    l_in_->setZValue(100);

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

void ImageView::setImage(const QImage& img)
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

        qint64 displayTs = FrameClock::nowNs();

        emit frameDisplayed(lastReceiveTs_,
                            displayTs);

        return;
    }

    const qint64 elapsed = displayTimer.elapsed();

    if (elapsed >= minDisplayIntervalMs)
    {
        updatePixmap(pendingFrame);
        hasPendingFrame = false;
        displayTimer.restart();

        qint64 displayTs = FrameClock::nowNs();

        emit frameDisplayed(lastReceiveTs_,
                            displayTs);
    }
    else
    {
        if (!throttleTimer->isActive())
        {
            const qint64 remaining = minDisplayIntervalMs - elapsed;

            if (remaining > 0)
            {
                const int interval =
                    static_cast<int>(std::min<qint64>(
                        remaining,
                        std::numeric_limits<int>::max()));

                throttleTimer->start(interval);
            }
        }
    }
}

void ImageView::setContour(const QVector<QPoint>& l_out,
                           const QVector<QPoint>& l_in)
{
    if ( !l_out_ || !l_in_ )
        return;

    l_out_->setPoints(l_out);
    l_in_->setPoints(l_in);
}

void ImageView::setImageAndContour(const QImage& image,
                                   const QVector<QPoint>& l_out,
                                   const QVector<QPoint>& l_in,
                                   qint64 receiveTs)
{
    setImage(image);
    setContour(l_out, l_in);

    lastReceiveTs_ = receiveTs;

    update();
}

void ImageView::clearOverlays()
{
    if ( l_out_ )
        l_out_->clearPoints();

    if ( l_in_ )
        l_in_->clearPoints();
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

    qint64 displayTs = FrameClock::nowNs();

    emit frameDisplayed(lastReceiveTs_,
                        displayTs);
}

void ImageView::updatePixmap(const QImage& img)
{
    if ( img.isNull() || !pixmapItem )
        return;

    bool sizeChanged =
        ( lastDisplayedImage.size() != img.size() );


    pixmapItem->setPixmap(QPixmap::fromImage(img));
    lastDisplayedImage = img;

    scene->setSceneRect(0, 0, img.width(), img.height());

    if (sizeChanged)
    {
        updateDisplayWithConfig();
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

    if (m_interaction)
        setCursor(m_interaction->cursorForEvent(*this,
                                                hasImage(),
                                                isPanRelevant(),
                                                nullptr));

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
    {
        m_interaction->mousePress(*this, event);
        setCursor(m_interaction->cursorForEvent(*this,
                                                hasImage(),
                                                isPanRelevant(),
                                                event));
    }

    if ( !event->isAccepted() )
        QGraphicsView::mousePressEvent(event);
}

void ImageView::mouseMoveEvent(QMouseEvent* event)
{
    if ( m_interaction )
    {
        m_interaction->mouseMove(*this, event);
        setCursor(m_interaction->cursorForEvent(*this,
                                                hasImage(),
                                                isPanRelevant(),
                                                event));
    }

    if ( !event->isAccepted() )
        QGraphicsView::mouseMoveEvent(event);
}

void ImageView::mouseReleaseEvent(QMouseEvent* event)
{
    if ( m_interaction )
    {
        m_interaction->mouseRelease(*this, event);
        setCursor(m_interaction->cursorForEvent(*this,
                                                hasImage(),
                                                isPanRelevant(),
                                                event));
    }

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
    if (!pixmapItem || lastDisplayedImage.isNull())
        return QPoint(-1, -1);

    const QPointF scenePos = mapToScene(viewPos);

    QPointF itemPos = pixmapItem->mapFromScene(scenePos);

    const int x = static_cast<int>(std::floor(itemPos.x()));
    const int y = static_cast<int>(std::floor(itemPos.y()));
    const QPoint p(x, y);

    if ( !lastDisplayedImage.valid(p) )
        return QPoint(-1, -1);

    return p;
}

void ImageView::enterEvent(QEnterEvent*)
{
    if (m_interaction)
        setCursor(m_interaction->cursorForEvent(*this,
                                                hasImage(),
                                                isPanRelevant(),
                                                nullptr));
}

QRgb ImageView::pixelColorAt(const QPoint& imagePos) const
{
    if (!lastDisplayedImage.valid(imagePos))
        return QRgb();

    return lastDisplayedImage.pixel(imagePos);
}

void ImageView::setListener(ImageViewListener* listener)
{
    listener_ = listener;
}

void ImageView::onColorPicked(const QColor& color,
                              const QPoint& imagePos)
{
    if (listener_)
        listener_->onColorPicked(color, imagePos);
}

const QImage& ImageView::image() const
{
    return lastDisplayedImage;
}

QImage ImageView::renderToImage() const
{
    if ( !scene || scene->sceneRect().isEmpty() )
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

bool ImageView::hasImage() const
{
    return pixmapItem != nullptr && !lastDisplayedImage.isNull();
}

bool ImageView::isPanRelevant() const
{
    if (!hasImage())
        return false;

    const QRectF sceneRect = scene->sceneRect();
    const QRectF viewRect  = mapToScene(viewport()->rect()).boundingRect();

    return sceneRect.width()  > viewRect.width()
           || sceneRect.height() > viewRect.height();
}

QGraphicsScene* ImageView::graphicsScene() const
{
    return scene;
}

bool ImageView::isGrayscale() const
{
    return lastDisplayedImage.format() == QImage::Format_Grayscale8 ||
           lastDisplayedImage.format() == QImage::Format_Grayscale16;
}

void ImageView::upscaleItems()
{
    clearOverlays();

    const bool has_ds = downscaleConfig_.hasDownscale;
    const int      df = downscaleConfig_.downscaleFactor;

    qreal factor = static_cast<qreal>(1.0);

    if ( has_ds && displayConfig_.input_displayed )
        factor = qreal( df );

    l_out_->setScale( factor );
    l_in_->setScale( factor );
}

void ImageView::applyDisplayConfig(const DisplayConfig& display)
{
    displayConfig_ = display;

    updateDisplayWithConfig();
}

void ImageView::updateDisplayWithConfig()
{
    QColor col_lout, col_lin;

    if ( displayConfig_.l_out_displayed )
        col_lout = toQColor(displayConfig_.l_out_color);
    else
        col_lout = Qt::transparent;

    if ( displayConfig_.l_in_displayed )
        col_lin = toQColor(displayConfig_.l_in_color);
    else
        col_lin = Qt::transparent;

    l_out_->setColor( col_lout );
    l_in_->setColor( col_lin );

    upscaleItems();

    updateFlip();
}

void ImageView::updateFlip()
{
    if (!pixmapItem)
        return;

    contentRoot_->setTransformOriginPoint(0, 0);

    if (displayConfig_.mirrorMode)
    {
        QTransform t;
        t.scale(-1, 1);
        contentRoot_->setTransform(t);

        contentRoot_->setPos(pixmapItem->pixmap().width(), 0);
    }
    else
    {
        contentRoot_->setTransform(QTransform());
        contentRoot_->setPos(0, 0);
    }
}

void ImageView::applyDownscaleConfig(const DownscaleConfig& downscale)
{
    downscaleConfig_ = downscale;

    upscaleItems();
}

} // namespace ofeli_app
