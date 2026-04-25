// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "application_settings_types.hpp"
#include "contour_point_item.hpp"
#include "frame_data.hpp"
#include "image_viewer_listener.hpp"
#include "overlay_text_item.hpp"

#include <QElapsedTimer>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QImage>
#include <QTimer>

class QWheelEvent;
class QMouseEvent;
class QResizeEvent;
class QGraphicsScene;

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

namespace fluvel_app
{

class ImageViewerInteraction;
class ZoomOverlayController;

/**
 * @brief Image viewer widget with interaction and overlay support.
 *
 * This widget provides a complete image viewing component based on QGraphicsView,
 * including:
 * - image display and scaling
 * - contour rendering
 * - user interactions (zoom, pan, behaviors)
 * - overlays (text, pixel info, zoom feedback)
 * - optional frame throttling for controlled display rate
 *
 * Architecture:
 * - Rendering is handled via a QGraphicsScene (pixmap + items)
 * - Interactions are delegated to an ImageViewerInteraction
 * - External callbacks are forwarded through ImageViewerListener
 *
 * The widget supports both direct image updates and frame-based updates
 * (UiFrame), with optional throttling for performance control.
 *
 * @note Interaction and listener are not owned by this class.
 *       Their lifetime must exceed the widget.
 */
class ImageViewerWidget : public QGraphicsView
{
    Q_OBJECT

public:
    // --- Public API (external users) ---
    explicit ImageViewerWidget(QWidget* parent = nullptr);

    explicit ImageViewerWidget(const DisplayConfig& displayConfig, const DownscaleParams& downscaleParams,
                       QWidget* parent = nullptr);

    ~ImageViewerWidget() override;

    /**
     * @brief Sets the interaction handler.
     *      * @param interaction Interaction object (not owned).
     */
    void setInteraction(ImageViewerInteraction* interaction);

    /**
     * @brief Sets the listener for external callbacks.
     *      * @param listener Listener object (not owned).
     */
    void setListener(ImageViewerListener* listener);

    /**
     * @brief Sets the maximum display frame rate.
     *      * If fps > 0, frames are throttled to the specified rate.
     * If fps == 0, throttling is disabled.
     *      * @param fps Maximum frames per second.
     */
    void setMaxDisplayFps(double fps);

    /**
     * @name Configuration
     * @brief Runtime configuration of display and processing parameters.
     *      * These functions update internal rendering behavior without recreating the widget.
     * @{
     */
    void applyDisplayConfig(const fluvel_app::DisplayConfig& display);
    void applyDownscaleConfig(const fluvel_app::DownscaleParams& downscale);
    /** @} */

    /**
     * @name Display
     * @brief Functions related to image and overlay display.
     *      * These functions update what is rendered in the viewer,
     * including image content, contours and UI overlays.
     * @{
     */

    /**
     * @brief Sets the current image.
     */
    void setImage(const QImage& img);

    /**
     * @brief Sets contour overlays.
     */
    void setContour(const QVector<QPointF>& outerContour, const QVector<QPointF>& innerContour);

    /**
     * @brief Sets both image and contour from a frame.
     */
    void setImageAndContour(const UiFrame& uiFrame);

    /**
     * @brief Clears contour overlays.
     */
    void clearContour();

    /**
     * @brief Displays a text overlay.
     */
    void setText(const QString& text);

    /**
     * @brief Shows or hides a placeholder effect.
     */
    void showPlaceholder(bool showEffect);
    /** @} */

    /**
     * @brief Returns the current image.
     */
    const QImage& image() const;

    /**
     * @brief Renders the current view to an image.
     */
    QImage renderToImage() const;

    /**
     * @name Internal API
     * @brief Functions used by behaviors and interaction system.
     * @{
     */

    void toggleFullscreen();
    void applyAutoFit();

    bool isPanRelevant() const;
    void userInteracted();
    void translateView(double dx, double dy);
    void scaleView(double sx, double sy);

    QPoint imageCoordinatesFromView(const QPoint& viewPos) const;
    QRgb pixelColorAt(const QPoint& imagePos) const;
    bool isPixelVisible(const QPoint& viewPos) const;
    bool isGrayscale() const;

    void onColorPicked(const QColor& color, const QPoint& imagePos);

    bool hasImage() const;

    void setDragHighlight(bool enabled);
    void notifyImageDropped(const QString& path);

    /** @} */

signals:
    /**
     * @brief Emitted when the image is clicked.
     */
    void imageClicked(int x, int y);

    /**
     * @brief Emitted when a frame is displayed.
     */
    void frameDisplayed(const fluvel_app::FrameTimestamps& ts);

    /**
     * @brief Emitted when an image is dropped.
     */
    void imageDropped(const QString& path);

protected:
    void wheelEvent(QWheelEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    bool viewportEvent(QEvent* event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;

    void enterEvent(QEnterEvent*) override;

    void resizeEvent(QResizeEvent* event) override;

private:
    // internal init
    void initializeView();

    void setupView();
    void setupScene();
    void setupItems();
    void setupGlobalOverlays();
    void setupInfoOverlay();
    void setupContourItems();
    void setupTimers();

    // internal image display
    void submitFrame(const UiFrame& frame);
    bool shouldDisplayImmediately() const;
    void schedulePendingFrame();
    void displayPendingFrame();

    void flushPendingFrame();
    void displayFrameNow(const UiFrame& f);
    void updatePixmap(const QImage& img);
    void updatePixmapItem(const QImage& img);
    void updateSceneRect(const QImage& img);
    void handleImageSizeChanged();

    // interactions
    bool handleInteractionWheel(QWheelEvent* event);
    double computeZoomFactor(QWheelEvent* event) const;
    bool applyZoom(QWheelEvent* event, double factor);
    void updateOverlays(const QPoint& cursorPosition, const QPoint& textPosition);
    void updateInteractionAfterZoom();

    QPoint textPosition(const OverlayTextItem* textOverlay) const;
    void setTextPosition(QPoint position, OverlayTextItem* textOverlay);

    void updateCursor(const QMouseEvent* e);
    double currentZoom() const;

    void updateDisplayWithConfig();
    void updateContourColors();
    void upscaleItems();
    void updateFlip();
    void updateSmoothDisplay();
    void updateTextOverlayVisibility();

    bool supportsDragDrop() const;

    QGraphicsScene* scene_ = nullptr;
    QGraphicsItemGroup* contentRoot_ = nullptr;
    QGraphicsPixmapItem* pixmapItem_ = nullptr;
    QImage lastDisplayedImage_;

    DisplayConfig displayConfig_;
    DownscaleParams downscaleConfig_;

    ContourPointsItem* outerContour_ = nullptr;
    ContourPointsItem* innerContour_ = nullptr;

    OverlayTextItem* infoOverlay_ = nullptr;

    OverlayTextItem* zoomOverlayItem_ = nullptr;
    ZoomOverlayController* zoomOverlayController_ = nullptr;

    ImageViewerInteraction* interaction_ = nullptr;
    ImageViewerListener* listener_ = nullptr;

    // --- Throttling ---
    UiFrame pendingFrame_;
    bool hasPendingFrame_{false};

    QElapsedTimer displayTimer_;
    int minDisplayIntervalMs_{0};

    QTimer* throttleTimer_ = nullptr;

    bool autoFitEnabled_{true};

    // --- Zoom / Pan ---
    const double minZoom_{0.1};
    const double maxZoom_{20.0};

    // --- Fullscreen ---
    bool isFullScreenMode_{false};
    QRect normalGeometry_;
    Qt::WindowFlags normalWindowFlags_;

    QGraphicsBlurEffect* blur_ = nullptr;
    bool placeholderVisible_{false};

    bool dragHighlight_{false};

    const bool useEnhancedDisplayConfig_{false};
};

} // namespace fluvel_app
