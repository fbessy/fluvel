#include "image_view.hpp"
#include "image_view_interaction.hpp"
#include "application_settings.hpp"
#include "common_settings.hpp"
#include "color_adapters.hpp"
#include "frame_clock.hpp"
#include "overlay_text_item.hpp"

#include <cassert>

#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QThread>

namespace ofeli_app
{

ImageView::ImageView(QWidget* parent)
    : QGraphicsView(parent)
{
    initialize();
}

ImageView::ImageView(const DisplayConfig& displayConfig,
                     const DownscaleConfig& downscaleConfig,
                     QWidget* parent)
    : QGraphicsView(parent),
    displayConfig_(displayConfig),
    downscaleConfig_(downscaleConfig)
{
    initialize();

    l_out_ = new ContourPointsItem(contentRoot_);
    l_out_->setZValue(100.0);

    l_in_ = new ContourPointsItem(contentRoot_);
    l_in_->setZValue(100.0);

    overlay_ = new OverlayTextItem;
    scene->addItem(overlay_);
    overlay_->setZValue(100.0);
    overlay_->hide();
}

void ImageView::initialize()
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
    updateSmoothDisplay();

    blur_ = new QGraphicsBlurEffect;
    pixmapItem->setGraphicsEffect(blur_);
    blur_->setBlurRadius(0);

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

void ImageView::setContour(const QVector<QPointF>& l_out,
                           const QVector<QPointF>& l_in)
{
    assert( l_out_ && l_in_ );

    l_out_->setPoints(l_out);
    l_in_->setPoints(l_in);
}

void ImageView::setImageAndContour(const QImage& image,
                                   const QVector<QPointF>& l_out,
                                   const QVector<QPointF>& l_in,
                                   qint64 receiveTs)
{
    assert( l_out_ && l_in_ );

    setImage(image);
    setContour(l_out, l_in);

    lastReceiveTs_ = receiveTs;
}

void ImageView::clearOverlays()
{
    assert( l_out_ && l_in_ );

    l_out_->clearPoints();
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

    scene->setSceneRect( 0.0,
                         0.0,
                         static_cast<qreal>(img.width()),
                         static_cast<qreal>(img.height()) );

    if (sizeChanged)
    {
        setTextPosition( { 10, 20 } );

        if ( l_out_ && l_in_ )
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
    const QPoint overlayPosition = textPosition();

    scale(sx, sy);

    setTextPosition(overlayPosition);
}

void ImageView::translateView(double dx, double dy)
{
    const QPoint overlayPosition = textPosition();

    translate(dx, dy);

    setTextPosition(overlayPosition);
}

double ImageView::getCurrentZoom() const
{
    return transform().m11(); // scale X
}

void ImageView::toggleFullscreen()
{
    const QPoint overlayPosition = textPosition();

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

    setTextPosition(overlayPosition);
}

void ImageView::applyAutoFit()
{
    const QPoint overlayPosition = textPosition();

    if (!pixmapItem)
        return;

    autoFitEnabled = true;
    resetTransform();
    fitInView(pixmapItem, Qt::KeepAspectRatio);

    setTextPosition(overlayPosition);
}

void ImageView::userInteracted()
{
    autoFitEnabled = false;
}

QPoint ImageView::textPosition() const
{
    QPoint overlayPosition = { 0, 0 };

    if ( overlay_ )
    {
        overlayPosition = mapFromScene(overlay_->pos());

        if (    overlayPosition.x() < 0 || overlayPosition.x() >= width()
             || overlayPosition.y() < 0 || overlayPosition.y() >= height() )
        {
            overlayPosition = { 10, 20 };
        }
    }

    return overlayPosition;
}

void ImageView::setTextPosition(QPoint position)
{
    if ( overlay_ )
        overlay_->setPos(mapToScene(position));
}

// ------------------------------------------------------------
// Interaction
// ------------------------------------------------------------
void ImageView::wheelEvent(QWheelEvent* event)
{
    const QPoint overlayPosition = textPosition();

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

    setTextPosition(overlayPosition);
}

void ImageView::resizeEvent(QResizeEvent* event)
{
    const QPoint overlayPosition = textPosition();

    QGraphicsView::resizeEvent(event);

    if (autoFitEnabled)
        applyAutoFit();


    setTextPosition(overlayPosition);
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
    // 1️⃣ Cherche l’item le plus haut sous la souris (respecte le Z)
    QGraphicsItem* itemUnderMouse = itemAt(event->pos());

    // 2️⃣ Si on clique sur un item draggable, on laisse Qt gérer
    if (itemUnderMouse &&
        (itemUnderMouse->flags() & QGraphicsItem::ItemIsMovable))
    {
        QGraphicsView::mousePressEvent(event);
        return;
    }

    // 3️⃣ Sinon, on laisse l'interaction manager décider
    bool handled = false;

    if (m_interaction)
    {
        handled = m_interaction->mousePress(*this, event);

        setCursor(m_interaction->cursorForEvent(
                  *this,
                  hasImage(),
                  isPanRelevant(),
                  event));
    }

    // 4️⃣ Si personne n’a géré, on passe à Qt
    if (!handled)
        QGraphicsView::mousePressEvent(event);
}

void ImageView::mouseMoveEvent(QMouseEvent* event)
{
    // 1️⃣ Cherche l’item le plus haut sous la souris (respecte le Z)
    QGraphicsItem* itemUnderMouse = itemAt(event->pos());

    // 2️⃣ Si on clique sur un item draggable, on laisse Qt gérer
    if (itemUnderMouse &&
        (itemUnderMouse->flags() & QGraphicsItem::ItemIsMovable))
    {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    // 3️⃣ Sinon, on laisse l'interaction manager décider
    bool handled = false;

    if (m_interaction)
    {
        handled = m_interaction->mouseMove(*this, event);

        setCursor(m_interaction->cursorForEvent(
            *this,
            hasImage(),
            isPanRelevant(),
            event));
    }

    // 4️⃣ Si personne n’a géré, on passe à Qt
    if (!handled)
        QGraphicsView::mouseMoveEvent(event);
}

void ImageView::mouseReleaseEvent(QMouseEvent* event)
{
    // 1️⃣ Cherche l’item le plus haut sous la souris (respecte le Z)
    QGraphicsItem* itemUnderMouse = itemAt(event->pos());

    // 2️⃣ Si on clique sur un item draggable, on laisse Qt gérer
    if (itemUnderMouse &&
        (itemUnderMouse->flags() & QGraphicsItem::ItemIsMovable))
    {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }

    // 3️⃣ Sinon, on laisse l'interaction manager décider
    bool handled = false;

    if (m_interaction)
    {
        handled = m_interaction->mouseRelease(*this, event);

        setCursor(m_interaction->cursorForEvent(
            *this,
            hasImage(),
            isPanRelevant(),
            event));
    }

    // 4️⃣ Si personne n’a géré, on passe à Qt
    if (!handled)
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

    const int x = static_cast<int>(itemPos.x());
    const int y = static_cast<int>(itemPos.y());
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
    assert( l_out_ && l_in_ );

    const bool has_ds = downscaleConfig_.hasDownscale;
    const int      df = downscaleConfig_.downscaleFactor;

    qreal factor = 1.0;

    if ( has_ds &&
         displayConfig_.image == ImageBase::Source )
        factor = qreal( df );

    l_out_->setScale( factor );
    l_in_->setScale( factor );
}

void ImageView::applyDisplayConfig(const DisplayConfig& display)
{
    assert( l_out_ && l_in_ );

    displayConfig_ = display;

    updateDisplayWithConfig();
}

void ImageView::updateDisplayWithConfig()
{
    updateContourColors();
    upscaleItems();
    updateFlip();
    updateSmoothDisplay();
    updateTextOverlayVisibility();
}

void ImageView::updateContourColors()
{
    assert( l_out_ && l_in_ );

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
}

void ImageView::updateFlip()
{
    assert( l_out_ && l_in_ );

    contentRoot_->setTransformOriginPoint(0.0, 0.0);

    if (displayConfig_.mirrorMode)
    {
        QTransform t;
        t.scale(-1.0, 1.0);
        contentRoot_->setTransform(t);

        contentRoot_->setPos(static_cast<qreal>(pixmapItem->pixmap().width()),
                             0.0);
    }
    else
    {
        contentRoot_->setTransform(QTransform());
        contentRoot_->setPos(0.0, 0.0);
    }
}

void ImageView::updateSmoothDisplay()
{
    if ( !pixmapItem )
        return;

    if ( displayConfig_.smoothDisplay )
        pixmapItem->setTransformationMode(Qt::SmoothTransformation);
    else
        pixmapItem->setTransformationMode(Qt::FastTransformation);
}

void ImageView::updateTextOverlayVisibility()
{
    if ( !overlay_ )
        return;

    overlay_->setVisible(displayConfig_.algorithm_overlay);
}

void ImageView::applyDownscaleConfig(const DownscaleConfig& downscale)
{
    assert( l_out_ && l_in_ );

    downscaleConfig_ = downscale;

    upscaleItems();
}

void ImageView::setText(const QString& text)
{
    if ( overlay_ )
        overlay_->setText(text);
}

QColor ImageView::desaturateAndDarken(const QColor& original,
                                      qreal saturationFactor,
                                      qreal valueFactor)
{
    // Clamp des facteurs pour éviter les aberrations
    saturationFactor = std::clamp(saturationFactor, 0.0, 1.0);
    valueFactor      = std::clamp(valueFactor,      0.0, 1.0);

    QColor hsv = original.toHsv();

    int h = hsv.hue();         // peut être -1 si gris
    int s = hsv.saturation();  // 0 → gris pur
    int v = hsv.value();
    int a = hsv.alpha();

    // Si gris pur → on ne touche qu'à la luminosité
    if (s == 0)
    {
        v = static_cast<int>(v * valueFactor);
        return QColor::fromHsv(0, 0, v, a);
    }

    // Couleur normale
    s = static_cast<int>(s * saturationFactor);
    v = static_cast<int>(v * valueFactor);

    return QColor::fromHsv(h, s, v, a);
}

QImage ImageView::darkenImage(const QImage& image)
{
    QImage dark = image.convertToFormat(QImage::Format_ARGB32);

    QPainter p(&dark);
    p.setCompositionMode(QPainter::CompositionMode_Multiply);

    // gris sombre plutôt que noir pur
    p.fillRect(dark.rect(), QColor(100,100,100));
    p.end();

    return dark;
}

void ImageView::showPlaceholder(bool showEffect)
{
    if ( !blur_ )
        return;

    if ( showEffect )
    {
        setImage(darkenImage( lastDisplayedImage) );

        pixmapItem->update();
        contentRoot_->update();
        scene->update();

        if ( l_out_ )
            l_out_->setColor( desaturateAndDarken(toQColor(displayConfig_.l_out_color),
                                                 0.4, 0.6) );

        if ( l_in_ )
            l_in_->setColor( desaturateAndDarken(toQColor(displayConfig_.l_in_color),
                                                0.4, 0.6) );
    }
    else
    {
        updateContourColors();
    }

    blur_->setBlurRadius(showEffect ? 6 : 0);
}

} // namespace ofeli_app
