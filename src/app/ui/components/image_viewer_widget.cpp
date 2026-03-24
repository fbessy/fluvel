// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "image_viewer_widget.hpp"
#include "color_adapters.hpp"
#include "common_settings.hpp"
#include "drag_drop_behavior.hpp"
#include "frame_clock.hpp"
#include "image_viewer_interaction.hpp"
#include "interaction_set.hpp"
#include "overlay_text_item.hpp"
#include "qcolor_utils.hpp"
#include "qimage_utils.hpp"
#include "zoom_overlay_controller.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QThread>
#include <QTimer>
#include <QWheelEvent>

#include <cassert>

namespace fluvel_app
{

ImageViewerWidget::ImageViewerWidget(QWidget* parent)
    : QGraphicsView(parent)
{
    initializeView();
}

ImageViewerWidget::ImageViewerWidget(const DisplayConfig& displayConfig,
                                     const DownscaleConfig& downscaleConfig, QWidget* parent)
    : QGraphicsView(parent)
    , displayConfig_(displayConfig)
    , downscaleConfig_(downscaleConfig)
    , useEnhancedDisplayConfig_{true}
{
    initializeView();

    setupContourItems();
    setupInfoOverlay();

    updateSmoothDisplay();
}

void ImageViewerWidget::initializeView()
{
    setupView();
    setupScene();
    setupItems();
    setupGlobalOverlays();
    setupTimers();
}

void ImageViewerWidget::setupView()
{
    setRenderHint(QPainter::SmoothPixmapTransform, false);
    setAlignment(Qt::AlignCenter);

    setTransformationAnchor(QGraphicsView::NoAnchor);
    setResizeAnchor(QGraphicsView::NoAnchor);
    setDragMode(QGraphicsView::NoDrag);
    setAcceptDrops(true);

    setMouseTracking(true);
    viewport()->setMouseTracking(true);
}

void ImageViewerWidget::setupScene()
{
    scene_ = new QGraphicsScene(this);
    setScene(scene_);

    contentRoot_ = new QGraphicsItemGroup;
    scene_->addItem(contentRoot_);
}

void ImageViewerWidget::setupItems()
{
    pixmapItem_ = new QGraphicsPixmapItem(contentRoot_);
    pixmapItem_->setZValue(0);

    blur_ = new QGraphicsBlurEffect;
    pixmapItem_->setGraphicsEffect(blur_);
    blur_->setBlurRadius(0);
}

void ImageViewerWidget::setupGlobalOverlays()
{
    zoomOverlayItem_ = new OverlayTextItem;
    scene_->addItem(zoomOverlayItem_);

    zoomOverlayController_ = new ZoomOverlayController(zoomOverlayItem_, this);
}

void ImageViewerWidget::setupInfoOverlay()
{
    infoOverlay_ = new OverlayTextItem;
    scene_->addItem(infoOverlay_);

    infoOverlay_->setZValue(100.0);
    infoOverlay_->hide();
    infoOverlay_->setCursor(Qt::ArrowCursor);
}

void ImageViewerWidget::setupContourItems()
{
    outerContour_ = new ContourPointsItem(contentRoot_);
    outerContour_->setZValue(100.0);

    innerContour_ = new ContourPointsItem(contentRoot_);
    innerContour_->setZValue(100.0);
}

void ImageViewerWidget::setupTimers()
{
    displayTimer_.start();

    throttleTimer_ = new QTimer(this);
    throttleTimer_->setSingleShot(true);

    connect(throttleTimer_, &QTimer::timeout, this, &ImageViewerWidget::flushPendingFrame);
}

// ------------------------------------------------------------
// Public API
// ------------------------------------------------------------
void ImageViewerWidget::setMaxDisplayFps(double fps)
{
    if (fps <= 0.0)
    {
        minDisplayIntervalMs_ = 0;
        return;
    }

    minDisplayIntervalMs_ = static_cast<int>(1000.0 / fps);
}

void ImageViewerWidget::displayFrameNow(const UiFrame& f)
{
    updatePixmap(f.image);

    if (!f.outerContour.isEmpty() || !f.innerContour.isEmpty())
        setContour(f.outerContour, f.innerContour);

    displayTimer_.restart();

    QTimer::singleShot(0, this,
                       [this, f]()
                       {
                           qint64 displayTs = FrameClock::nowNs();

                           emit frameDisplayed(FrameTimestamps{f.receiveTimestampNs,
                                                               f.processTimestampNs, displayTs});
                       });
}

void ImageViewerWidget::submitFrame(const UiFrame& frame)
{
    pendingFrame_ = frame;
    hasPendingFrame_ = true;

    if (shouldDisplayImmediately())
    {
        displayPendingFrame();
        return;
    }

    schedulePendingFrame();
}

bool ImageViewerWidget::shouldDisplayImmediately() const
{
    if (minDisplayIntervalMs_ == 0)
        return true;

    return displayTimer_.elapsed() >= minDisplayIntervalMs_;
}

void ImageViewerWidget::schedulePendingFrame()
{
    if (throttleTimer_->isActive())
        return;

    const qint64 elapsed = displayTimer_.elapsed();
    const qint64 remaining = minDisplayIntervalMs_ - elapsed;

    if (remaining > 0)
    {
        throttleTimer_->start(
            static_cast<int>(std::min<qint64>(remaining, std::numeric_limits<int>::max())));
    }
}

void ImageViewerWidget::displayPendingFrame()
{
    displayFrameNow(pendingFrame_);
    hasPendingFrame_ = false;
}

void ImageViewerWidget::setImage(const QImage& img)
{
    UiFrame f;
    f.image = img;

    qint64 now = FrameClock::nowNs();
    f.receiveTimestampNs = now;
    f.processTimestampNs = now;

    submitFrame(f);
}

void ImageViewerWidget::setContour(const QVector<QPointF>& outerContour,
                                   const QVector<QPointF>& innerContour)
{
    assert(outerContour_ && innerContour_);

    outerContour_->setPoints(outerContour);
    innerContour_->setPoints(innerContour);
}

void ImageViewerWidget::setImageAndContour(const UiFrame& frame)
{
    submitFrame(frame);
}

void ImageViewerWidget::clearContour()
{
    assert(outerContour_ && innerContour_);

    outerContour_->clearPoints();
    innerContour_->clearPoints();
}

// ------------------------------------------------------------
// Internal rendering
// ------------------------------------------------------------
void ImageViewerWidget::flushPendingFrame()
{
    if (!hasPendingFrame_)
        return;

    displayFrameNow(pendingFrame_);
    hasPendingFrame_ = false;
}

void ImageViewerWidget::updatePixmap(const QImage& img)
{
    assert(pixmapItem_);

    if (img.isNull())
        return;

    const bool sizeChanged = (lastDisplayedImage_.size() != img.size());

    updatePixmapItem(img);
    updateSceneRect(img);

    if (sizeChanged)
        handleImageSizeChanged();
}

void ImageViewerWidget::updatePixmapItem(const QImage& img)
{
    assert(pixmapItem_);

    pixmapItem_->setPixmap(QPixmap::fromImage(img));
    lastDisplayedImage_ = img;
}

void ImageViewerWidget::updateSceneRect(const QImage& img)
{
    assert(scene_);

    scene_->setSceneRect(0.0, 0.0, static_cast<qreal>(img.width()),
                         static_cast<qreal>(img.height()));
}

void ImageViewerWidget::handleImageSizeChanged()
{
    if (infoOverlay_)
    {
        constexpr QPoint kDefaultOverlayPos{10, 20};
        setTextPosition(kDefaultOverlayPos, infoOverlay_);
    }

    if (useEnhancedDisplayConfig_)
        updateDisplayWithConfig();

    applyAutoFit();
}

void ImageViewerWidget::scaleView(double sx, double sy)
{
    const QPoint overlayPosition = textPosition(infoOverlay_);

    scale(sx, sy);

    setTextPosition(overlayPosition, infoOverlay_);
}

void ImageViewerWidget::translateView(double dx, double dy)
{
    const QPoint overlayPosition = textPosition(infoOverlay_);

    translate(dx, dy);

    setTextPosition(overlayPosition, infoOverlay_);
}

double ImageViewerWidget::currentZoom() const
{
    return transform().m11(); // scale X
}

void ImageViewerWidget::toggleFullscreen()
{
    const QPoint overlayPosition = textPosition(infoOverlay_);

    if (!isFullScreenMode_)
    {
        normalGeometry_ = window()->geometry();
        normalWindowFlags_ = window()->windowFlags();
        window()->setWindowFlags(Qt::Window);
        window()->showFullScreen();

        isFullScreenMode_ = true;
    }
    else
    {
        window()->setWindowFlags(normalWindowFlags_);
        window()->showNormal();
        window()->setGeometry(normalGeometry_);
        isFullScreenMode_ = false;
    }

    setTextPosition(overlayPosition, infoOverlay_);
}

void ImageViewerWidget::applyAutoFit()
{
    const QPoint overlayPosition = textPosition(infoOverlay_);

    if (!pixmapItem_)
        return;

    autoFitEnabled_ = true;
    resetTransform();
    fitInView(pixmapItem_, Qt::KeepAspectRatio);

    setTextPosition(overlayPosition, infoOverlay_);
}

void ImageViewerWidget::userInteracted()
{
    autoFitEnabled_ = false;
}

QPoint ImageViewerWidget::textPosition(const OverlayTextItem* textOverlay) const
{
    QPoint overlayPosition = {0, 0};

    if (textOverlay)
    {
        overlayPosition = mapFromScene(textOverlay->pos());

        if (overlayPosition.x() < 0 || overlayPosition.x() >= width() || overlayPosition.y() < 0 ||
            overlayPosition.y() >= height())
        {
            overlayPosition = {10, 20};
        }
    }

    return overlayPosition;
}

void ImageViewerWidget::setTextPosition(QPoint position, OverlayTextItem* textOverlay)
{
    if (textOverlay)
        textOverlay->setPos(mapToScene(position));
}

// ------------------------------------------------------------
// Interaction
// ------------------------------------------------------------
void ImageViewerWidget::wheelEvent(QWheelEvent* event)
{
    if (!hasImage())
    {
        event->ignore();
        return;
    }

    if (handleInteractionWheel(event))
        return;

    double factor = computeZoomFactor(event);

    const QPoint textPos = textPosition(infoOverlay_);
    const QPoint cursorPos = event->position().toPoint();

    if (!applyZoom(event, factor))
        return;

    updateOverlays(cursorPos, textPos);
    updateInteractionAfterZoom();

    updateCursor(nullptr);
}

bool ImageViewerWidget::handleInteractionWheel(QWheelEvent* event)
{
    if ((event->modifiers() & Qt::ControlModifier) && interaction_)
    {
        if (interaction_->wheel(*this, event))
        {
            event->accept();
            return true;
        }
    }
    return false;
}

bool ImageViewerWidget::applyZoom(QWheelEvent* event, double factor)
{
    QPointF scenePosBefore = mapToScene(event->position().toPoint());

    double currZoom = currentZoom();
    double newZoom = currZoom * factor;

    if (newZoom < minZoom_ || newZoom > maxZoom_)
        return false;

    scale(factor, factor);

    QPointF scenePosAfter = mapToScene(event->position().toPoint());
    QPointF delta = scenePosAfter - scenePosBefore;
    translate(delta.x(), delta.y());

    autoFitEnabled_ = false;

#ifdef FLUVEL_DEBUG
    qDebug() << "mouseDelta =" << event->angleDelta().x() << event->angleDelta().y()
             << "zoom =" << currZoom << " new zoom =" << newZoom << "delta =" << delta.x()
             << delta.y();
#endif

    return true;
}

void ImageViewerWidget::updateOverlays(const QPoint& cursorPosition, const QPoint& textPosition)
{
    setTextPosition(textPosition, infoOverlay_);

    // zoom overlay
    double newZoom = currentZoom();
    int percent = static_cast<int>(std::round(newZoom * 100.0));

    zoomOverlayController_->show(percent);

    const QPoint zoomOverlayPosition = cursorPosition + QPoint(20, -20);

    setTextPosition(zoomOverlayPosition, zoomOverlayItem_);
}

void ImageViewerWidget::updateInteractionAfterZoom()
{
    if (!interaction_)
        return;

    QPoint pos = viewport()->mapFromGlobal(QCursor::pos());

    QPointF localPos = pos;
    QPointF scenePos = mapToScene(pos);
    QPointF globalPos = QCursor::pos();

    QMouseEvent fake(QEvent::MouseMove, localPos, scenePos, globalPos, Qt::NoButton,
                     Qt::RightButton, Qt::NoModifier);

    interaction_->mouseMove(*this, &fake);
}

double ImageViewerWidget::computeZoomFactor(QWheelEvent* event) const
{
    constexpr double zoomFactor = 1.15;
    return (event->angleDelta().y() > 0) ? zoomFactor : 1.0 / zoomFactor;
}

void ImageViewerWidget::resizeEvent(QResizeEvent* event)
{
    const QPoint overlayPosition = textPosition(infoOverlay_);

    QGraphicsView::resizeEvent(event);

    if (autoFitEnabled_)
        applyAutoFit();

    setTextPosition(overlayPosition, infoOverlay_);
}

void ImageViewerWidget::setInteraction(ImageViewerInteraction* interaction)
{
    interaction_ = interaction;
}

void ImageViewerWidget::mousePressEvent(QMouseEvent* event)
{
    QGraphicsItem* item = itemAt(event->pos());
    bool itemMovable = item && (item->flags() & QGraphicsItem::ItemIsMovable);

    if (!itemMovable && interaction_)
    {
        interaction_->mousePress(*this, event);
        updateCursor(event);
    }

    QGraphicsView::mousePressEvent(event);
}

void ImageViewerWidget::mouseMoveEvent(QMouseEvent* event)
{
    QGraphicsItem* item = itemAt(event->pos());
    bool itemMovable = item && (item->flags() & QGraphicsItem::ItemIsMovable);

    if (itemMovable)
    {
        if (interaction_)
            interaction_->cancel(); // 👈 stop pixel info

        QGraphicsView::mouseMoveEvent(event);
        return;
    }

    if (interaction_)
    {
        interaction_->mouseMove(*this, event);
        updateCursor(event);
    }

    QGraphicsView::mouseMoveEvent(event);
}

void ImageViewerWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QGraphicsItem* item = itemAt(event->pos());
    bool itemMovable = item && (item->flags() & QGraphicsItem::ItemIsMovable);

    if (!itemMovable && interaction_)
    {
        interaction_->mouseRelease(*this, event);
        updateCursor(event);
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void ImageViewerWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (interaction_)
        interaction_->mouseDoubleClick(*this, event);

    if (!event->isAccepted())
        QGraphicsView::mouseDoubleClickEvent(event);
}

void ImageViewerWidget::updateCursor(const QMouseEvent* e)
{
    if (!interaction_)
        return;

    QPoint pos = e ? e->pos() : viewport()->mapFromGlobal(QCursor::pos());

    const auto items = this->items(pos);

    // 1️⃣ priorité aux items UI déplaçables
    for (auto* item : items)
    {
        if (item->flags() & QGraphicsItem::ItemIsMovable)
        {
            viewport()->setCursor(Qt::ArrowCursor);
            return;
        }
    }

    if (!isPixelVisible(pos))
    {
        viewport()->setCursor(Qt::ArrowCursor);
        return;
    }

    // 3️⃣ sinon les interactions décident
    viewport()->setCursor(interaction_->cursorForEvent(*this, hasImage(), isPanRelevant(), e));
}

bool ImageViewerWidget::viewportEvent(QEvent* event)
{
    if (!interaction_)
        return QGraphicsView::viewportEvent(event);

    switch (event->type())
    {
        case QEvent::MouseMove:
        case QEvent::HoverMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        {
            auto* me = static_cast<QMouseEvent*>(event);

            updateCursor(me);

            break;
        }

        default:
            break;
    }

    return QGraphicsView::viewportEvent(event);
}

void ImageViewerWidget::dragEnterEvent(QDragEnterEvent* event)
{
    bool handled = false;

    if (interaction_)
        handled = interaction_->dragEnter(*this, event);

    if (!handled)
        event->ignore();
}

void ImageViewerWidget::dragMoveEvent(QDragMoveEvent* event)
{
    bool handled = false;

    if (interaction_)
        handled = interaction_->dragMove(*this, event);

    if (!handled)
        event->ignore();
}

void ImageViewerWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    if (interaction_)
        interaction_->dragLeave(*this, event);
}

void ImageViewerWidget::dropEvent(QDropEvent* event)
{
    bool handled = false;

    if (interaction_)
        handled = interaction_->drop(*this, event);

    if (!handled)
        event->ignore();
}

void ImageViewerWidget::drawForeground(QPainter* painter, const QRectF& rect)
{
    if (!supportsDragDrop())
        return;

    painter->save();

    if (dragHighlight_)
    {
        painter->fillRect(rect, QColor(100, 150, 255, 40));
    }

    if (!hasImage())
    {
        painter->setPen(QColor(150, 150, 150));

        QFont f = painter->font();
        f.setPointSize(f.pointSize() + 2);
        painter->setFont(f);

        painter->drawText(rect, Qt::AlignCenter, tr("Drop an image here\nor\nFile → Open"));
    }

    painter->restore();
}

QPoint ImageViewerWidget::imageCoordinatesFromView(const QPoint& viewPos) const
{
    if (!pixmapItem_ || lastDisplayedImage_.isNull())
        return QPoint(-1, -1);

    const QPointF scenePos = mapToScene(viewPos);

    QPointF itemPos = pixmapItem_->mapFromScene(scenePos);

    const int x = static_cast<int>(itemPos.x());
    const int y = static_cast<int>(itemPos.y());
    const QPoint p(x, y);

    if (!lastDisplayedImage_.valid(p))
        return QPoint(-1, -1);

    return p;
}

void ImageViewerWidget::enterEvent(QEnterEvent*)
{
    if (!interaction_)
        return;

    QPoint pos = viewport()->mapFromGlobal(QCursor::pos());

    QPointF localPos = pos;
    QPointF scenePos = mapToScene(pos);
    QPointF globalPos = QCursor::pos();

    QMouseEvent fake(QEvent::MouseMove, localPos, scenePos, globalPos, Qt::NoButton, Qt::NoButton,
                     Qt::NoModifier);

    updateCursor(&fake);
}

QRgb ImageViewerWidget::pixelColorAt(const QPoint& imagePos) const
{
    if (!lastDisplayedImage_.valid(imagePos))
        return QRgb();

    return lastDisplayedImage_.pixel(imagePos);
}

void ImageViewerWidget::setListener(ImageViewerListener* listener)
{
    listener_ = listener;
}

void ImageViewerWidget::onColorPicked(const QColor& color, const QPoint& imagePos)
{
    if (listener_)
        listener_->onColorPicked(color, imagePos);
}

const QImage& ImageViewerWidget::image() const
{
    return lastDisplayedImage_;
}

QImage ImageViewerWidget::renderToImage() const
{
    assert(scene_);

    if (scene_->sceneRect().isEmpty())
        return QImage();

    QRectF rect = scene_->sceneRect();

    QImage img(rect.size().toSize(), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    scene_->render(&painter, QRectF(img.rect()), rect, Qt::IgnoreAspectRatio);

    return img;
}

bool ImageViewerWidget::hasImage() const
{
    return pixmapItem_ != nullptr && !lastDisplayedImage_.isNull();
}

bool ImageViewerWidget::isPanRelevant() const
{
    if (!hasImage())
        return false;

    const QRectF sceneRect = scene_->sceneRect();
    const QRectF viewRect = mapToScene(viewport()->rect()).boundingRect();

    return sceneRect.width() > viewRect.width() || sceneRect.height() > viewRect.height();
}

bool ImageViewerWidget::isGrayscale() const
{
    return lastDisplayedImage_.format() == QImage::Format_Grayscale8 ||
           lastDisplayedImage_.format() == QImage::Format_Grayscale16;
}

void ImageViewerWidget::upscaleItems()
{
    assert(useEnhancedDisplayConfig_ && outerContour_ && innerContour_);

    const bool has_ds = downscaleConfig_.hasDownscale;
    const int df = downscaleConfig_.downscaleFactor;

    qreal factor = 1.0;

    if (has_ds && displayConfig_.image == ImageBase::Source)
        factor = qreal(df);

    outerContour_->setScale(factor);
    innerContour_->setScale(factor);
}

void ImageViewerWidget::applyDisplayConfig(const DisplayConfig& display)
{
    assert(useEnhancedDisplayConfig_);

    displayConfig_ = display;

    updateDisplayWithConfig();
}

void ImageViewerWidget::updateDisplayWithConfig()
{
    assert(useEnhancedDisplayConfig_);

    updateContourColors();
    upscaleItems();
    updateFlip();
    updateSmoothDisplay();
    updateTextOverlayVisibility();
}

void ImageViewerWidget::updateContourColors()
{
    assert(useEnhancedDisplayConfig_ && outerContour_ && innerContour_);

    QColor col_lout, col_lin;

    if (displayConfig_.l_out_displayed)
        col_lout = toQColor(displayConfig_.l_out_color);
    else
        col_lout = Qt::transparent;

    if (displayConfig_.l_in_displayed)
        col_lin = toQColor(displayConfig_.l_in_color);
    else
        col_lin = Qt::transparent;

    outerContour_->setColor(col_lout);
    innerContour_->setColor(col_lin);
}

void ImageViewerWidget::updateFlip()
{
    assert(useEnhancedDisplayConfig_);

    contentRoot_->setTransformOriginPoint(0.0, 0.0);

    if (displayConfig_.mirrorMode)
    {
        QTransform t;
        t.scale(-1.0, 1.0);
        contentRoot_->setTransform(t);

        contentRoot_->setPos(static_cast<qreal>(pixmapItem_->pixmap().width()), 0.0);
    }
    else
    {
        contentRoot_->setTransform(QTransform());
        contentRoot_->setPos(0.0, 0.0);
    }
}

void ImageViewerWidget::updateSmoothDisplay()
{
    assert(useEnhancedDisplayConfig_ && pixmapItem_);

    if (displayConfig_.smoothDisplay)
        pixmapItem_->setTransformationMode(Qt::SmoothTransformation);
    else
        pixmapItem_->setTransformationMode(Qt::FastTransformation);
}

void ImageViewerWidget::updateTextOverlayVisibility()
{
    if (!infoOverlay_)
        return;

    infoOverlay_->setVisible(displayConfig_.algorithm_overlay);
}

void ImageViewerWidget::applyDownscaleConfig(const DownscaleConfig& downscale)
{
    if (!useEnhancedDisplayConfig_)
        return;

    downscaleConfig_ = downscale;

    upscaleItems();
}

void ImageViewerWidget::setText(const QString& text)
{
    if (infoOverlay_)
        infoOverlay_->setText(text);
}

void ImageViewerWidget::showPlaceholder(bool showEffect)
{
    assert(blur_);

    if (placeholderVisible_ == showEffect)
        return;

    if (showEffect)
    {
        if (!lastDisplayedImage_.isNull())
            setImage(qimage_utils::darkenImage(lastDisplayedImage_));

        if (outerContour_)
            outerContour_->setColor(
                qcolor_utils::desaturateAndDarken(toQColor(displayConfig_.l_out_color), 0.4, 0.6));

        if (innerContour_)
            innerContour_->setColor(
                qcolor_utils::desaturateAndDarken(toQColor(displayConfig_.l_in_color), 0.4, 0.6));

        blur_->setBlurRadius(6);
    }
    else
    {
        updateContourColors();
        blur_->setBlurRadius(0);
    }

    placeholderVisible_ = showEffect;
}

void ImageViewerWidget::notifyImageDropped(const QString& path)
{
    emit imageDropped(path);
}

void ImageViewerWidget::setDragHighlight(bool enabled)
{
    dragHighlight_ = enabled;
    viewport()->update();
}

bool ImageViewerWidget::supportsDragDrop() const
{
    auto set = dynamic_cast<const InteractionSet*>(interaction_);

    return set && set->hasBehavior<DragDropBehavior>();
}

bool ImageViewerWidget::isPixelVisible(const QPoint& viewPos) const
{
    QPoint pixel = imageCoordinatesFromView(viewPos);
    if (pixel.x() < 0)
        return false;

    QPointF scenePos = mapToScene(viewPos);

    QRectF visibleSceneRect = mapToScene(viewport()->rect()).boundingRect();

    return visibleSceneRect.contains(scenePos);
}

} // namespace fluvel_app
