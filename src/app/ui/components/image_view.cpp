#include "image_view.hpp"
#include "image_view_interaction.hpp"
#include "application_settings.hpp"
#include "common_settings.hpp"

#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QThread>

namespace ofeli_app
{

ImageView::ImageView(QWidget* parent, Session session)
    : QGraphicsView(parent),
    scene(new QGraphicsScene(this)),
    display_config_(AppSettings::instance().imgSessSettings.img_disp_conf),
    downscale_config_(AppSettings::instance().imgSessSettings.downscale_conf),
    session_(session)
{
    setScene(scene);

    setRenderHint(QPainter::SmoothPixmapTransform, false);
    setAlignment(Qt::AlignCenter);

    setTransformationAnchor(QGraphicsView::NoAnchor);
    setResizeAnchor(QGraphicsView::NoAnchor);
    setDragMode(QGraphicsView::NoDrag);

    QColor col_lout, col_lin;

    if ( display_config_.l_out_displayed )
        col_lout = toQColor(display_config_.l_out_color);
    else
        col_lout = Qt::transparent;

    if ( display_config_.l_in_displayed )
        col_lin = toQColor(display_config_.l_in_color);
    else
        col_lin = Qt::transparent;

    contourOutItem = new ContourPointsItem;
    contourOutItem->setZValue(100);
    contourOutItem->setColor( col_lout );
    scene->addItem(contourOutItem);

    contourInItem = new ContourPointsItem;
    contourInItem->setZValue(100);
    contourInItem->setColor( col_lin );
    scene->addItem(contourInItem);

    applyDownscaleToItems();

    displayTimer.start();

    throttleTimer = new QTimer(this);
    throttleTimer->setSingleShot(true);

    connect(throttleTimer, &QTimer::timeout,
            this, &ImageView::flushPendingFrame);

    if ( session_ == Session::Image )
    {
        connect(&AppSettings::instance(), &ApplicationSettings::imgDisplaySettingsChanged,
                this, &ImageView::onConfigChanged);

        connect(&AppSettings::instance(), &ApplicationSettings::imgSettingsApplied,
                this, &ImageView::onDownscaleChanged);
    }
    else if ( session_ == Session::Camera )
    {
        connect(&AppSettings::instance(), &ApplicationSettings::camDisplaySettingsChanged,
                this, &ImageView::onConfigChanged);

        connect(&AppSettings::instance(), &ApplicationSettings::camSettingsApplied,
                this, &ImageView::onDownscaleChanged);
    }
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

    contourOutItem->setPoints(out);
    contourInItem->setPoints(in);
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
    return pixmapItem != nullptr;
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

void ImageView::onConfigChanged()
{
    if ( session_ == Session::Image )
        display_config_ = AppSettings::instance().imgSessSettings.img_disp_conf;
    else if ( session_ == Session::Camera )
        display_config_ = AppSettings::instance().camSessSettings.cam_disp_conf;

    QColor col_lout, col_lin;

    if ( display_config_.l_out_displayed )
        col_lout = toQColor(display_config_.l_out_color);
    else
        col_lout = Qt::transparent;

    if ( display_config_.l_in_displayed )
        col_lin = toQColor(display_config_.l_in_color);
    else
        col_lin = Qt::transparent;


    applyDownscaleToItems();

    contourOutItem->setColor( col_lout );
    contourInItem->setColor( col_lin );
}

void ImageView::onDownscaleChanged()
{
    if ( session_ == Session::Image )
        downscale_config_ = AppSettings::instance().imgSessSettings.downscale_conf;
    else if ( session_ == Session::Camera )
        downscale_config_ = AppSettings::instance().imgSessSettings.downscale_conf;


    clearOverlays();
    applyDownscaleToItems();
}

void ImageView::applyDownscaleToItems()
{
    const bool has_ds = downscale_config_.has_downscale;
    const int      df = downscale_config_.downscale_factor;

    qreal factor = static_cast<qreal>(1.0);

    if ( has_ds && display_config_.input_displayed )
        factor = qreal( df );

    contourOutItem->setScale( factor );
    contourInItem->setScale( factor );
}

} // namespace ofeli_app
