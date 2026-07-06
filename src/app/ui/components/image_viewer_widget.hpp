// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#ifndef Q_MOC_RUN
#include "application_settings_types.hpp"
#endif

#include "contour_point_item.hpp"
#include "frame_pipeline.hpp"
#include "image_viewer_listener.hpp"
#include "overlay_text_item.hpp"

#include <QElapsedTimer>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QImage>
#include <QTimer>
#include <QTransform>

class QWheelEvent;
class QMouseEvent;
class QResizeEvent;
class QGraphicsScene;

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

namespace fluvel
{

constexpr int kUserIdleTimeoutMs = 2000;

class ImageViewerInteraction;
class HudOverlayController;
class MiniMapWidget;

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
     *
     * @param interaction Interaction object (not owned).
     */
    void setInteraction(ImageViewerInteraction* interaction);

    /**
     * @brief Sets the listener for external callbacks.
     *
     * @param listener Listener object (not owned).
     */
    void setListener(ImageViewerListener* listener);

    /**
     * @brief Sets the maximum display frame rate.
     *
     * If fps > 0, frames are throttled to the specified rate.
     * If fps == 0, throttling is disabled.
     *
     * @param fps Maximum frames per second.
     */
    void setMaxDisplayFps(double fps);

    /**
     * @name Configuration
     * @brief Runtime configuration of display and processing parameters.
     *
* These functions update internal rendering behavior without recreating the widget.
     * @{
     */
    void applyDisplayConfig(const fluvel::DisplayConfig& display);
    void applyDownscaleConfig(const fluvel::DownscaleParams& downscale);
    /** @} */

    /**
     * @name Display
     * @brief Functions related to image and overlay display.
     *
     * These functions update what is rendered in the viewer,
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
     * @brief Sets the alignment of the overlay text.
     *
     * Controls how the text is positioned inside the overlay bounding box.
     * Typical values include:
     * - Qt::AlignCenter | Qt::AlignVCenter (HUD / video mode)
     * - Qt::AlignLeft   | Qt::AlignTop     (debug / image mode)
     *
     * @param align Qt alignment flags (combination of Qt::AlignmentFlag).
     */
    void setOverlayAlignment(Qt::Alignment align);

    /**
     * @brief Sets the minimum width of the overlay background.
     *
     * If set to 0, the overlay width adapts dynamically to the text content.
     * If greater than 0, the width is clamped to at least this value,
     * ensuring a stable layout.
     *
     * @param minW Minimum width in pixels.
     */
    void setOverlayMinWidth(qreal minW);

    /**
     * @brief Shows or hides a placeholder effect.
     */
    void showPlaceholder(bool showEffect);

    /**
     * @brief Returns the current image.
     */
    const QImage& image() const;

    /**
     * @brief Renders the current view to an image.
     */
    QImage renderToImage() const;

    /**
     * @brief Displays a temporary notification.
     *
     * Shows a transient HUD message using the notification preset.
     *
     * @param text Message to display.
     */
    void showNotification(const QString& text);

    /**
     * @brief Displays a temporary message near the mouse cursor.
     *
     * Shows a transient HUD message using the cursor preset.
     *
     * @param text Message to display.
     */
    void showCursorMessage(const QString& text);

    /**
     * @brief Displays the current zoom level.
     *
     * Shows a transient HUD message near the mouse cursor containing the
     * specified zoom percentage.
     *
     * @param percent Zoom level expressed as a percentage.
     */
    void showZoomHud(int percent);

    void positionCursorOverlay();

    /**
     * @brief Returns the displayed image rectangle in viewport coordinates.
     *
     * The returned rectangle corresponds to the visible image area inside
     * the viewport, after applying the current view transformation.
     *
     * This is useful for positioning UI elements relative to the displayed
     * image rather than the whole viewport (e.g. fullscreen controls or
     * image overlays).
     *
     * @return Displayed image rectangle in viewport coordinates.
     */
    QRect displayedImageRect() const;

    /**
     * @brief Adjusts an overlay position to keep it fully visible.
     *
     * The overlay is initially placed relative to an anchor point using the
     * preferred offset. If it would extend outside the viewport, its position
     * is adjusted to keep it fully visible.
     *
     * @param anchorViewPos Anchor position in viewport coordinates.
     * @param overlaySize Overlay size in viewport coordinates.
     * @param preferredOffset Preferred offset from the anchor position.
     * @return Adjusted position in viewport coordinates.
     */
    [[nodiscard]]
    QPoint adjustOverlayPosition(const QPoint& anchorViewPos, const QSize& overlaySize,
                                 const QPoint& preferredOffset) const;

    /**
     * @name Internal API
     * @brief Functions used by behaviors and interaction system.
     * @{
     */

    void toggleFullscreen();
    void enterFullscreenMode();
    void leaveFullscreenMode();

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

    void updateMiniMap();
    void positionMiniMap();
    void onMiniMapCenterRequested(const QPointF& scenePoint);
    void updateMiniMapThumbnail();

    bool isUserActive() const;
    bool isFullscreen() const;

    /** @} */

signals:
    /**
     * @brief Emitted when the image is clicked.
     */
    void imageClicked(int x, int y);

    /**
     * @brief Emitted when a frame is displayed.
     */
    void frameDisplayed(const fluvel::FrameTimestamps& ts);

    /**
     * @brief Emitted when an image is dropped.
     */
    void imageDropped(const QString& path);

    /**
     * @brief Emitted when the user requests toggling fullscreen mode.
     */
    void toggleFullscreenRequested();

    /**
     * @brief Emitted when user activity is detected.
     *
     * This signal is emitted whenever the user interacts with the
     * viewer (mouse movement, mouse button press, wheel event, etc.).
     *
     * It can be used to restore UI elements that were hidden after
     * a period of inactivity, such as fullscreen controls or the
     * mouse cursor.
     */
    void activityDetected(const QPoint& viewPos);

    /**
     * @brief Emitted when the viewer becomes idle.
     *
     * This signal is emitted after a configurable period of user
     * inactivity.
     *
     * It can be used to hide UI elements such as fullscreen controls
     * or the mouse cursor.
     */
    void idle();

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
    enum class OverlayPosition
    {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Center
    };

    // internal init
    void initializeView();

    void setupView();
    void setupScene();
    void setupItems();
    void setupGlobalOverlays();
    void setupMiniMap();
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
    void updateOverlays(const QPoint& overlayPosition);
    void updateInteractionAfterZoom();

    void updateCursor(const QMouseEvent* e);
    double currentZoom() const;

    void updateDisplayWithConfig();
    void updateContourColors();
    void upscaleItems();
    void updateFlip();
    void updateSmoothDisplay();
    void updateTextOverlayVisibility();
    void updateMiniMapVisibility();

    bool supportsDragDrop() const;

    void notifyUserActivity(const QPoint& viewPos);

    OverlayTextItem* createOverlayItem();

    /**
     * @brief Returns the current viewport position of an overlay.
     *
     * Converts the overlay scene position to viewport coordinates.
     *
     * @param overlay Overlay item.
     * @return Current overlay position in viewport coordinates.
     */
    QPoint overlayPosition(const OverlayTextItem* overlay) const;

    /**
     * @brief Moves an overlay to a specific viewport position.
     *
     * The specified position is expressed in viewport coordinates and is
     * internally converted to scene coordinates before moving the overlay.
     *
     * @param overlay Overlay item to move.
     * @param position Desired position in viewport coordinates.
     */
    void moveOverlay(OverlayTextItem* overlay, const QPoint& position);

    /**
     * @brief Anchors an overlay to a predefined viewport location.
     *
     * Positions the overlay relative to one of the predefined viewport
     * anchors (top-left, top-right, bottom-left, bottom-right or center).
     * An optional offset can be supplied to fine-tune the final placement.
     *
     * @param overlay Overlay item to position.
     * @param position Desired viewport anchor.
     * @param offset Additional offset applied after anchoring.
     */
    void anchorOverlay(OverlayTextItem* overlay,
                       OverlayPosition position = OverlayPosition::TopRight,
                       const QPointF& offset = {});

    QGraphicsScene* scene_{nullptr};
    QGraphicsItemGroup* contentRoot_{nullptr};
    QGraphicsPixmapItem* pixmapItem_{nullptr};
    QImage lastDisplayedImage_;

    DisplayConfig displayConfig_{};
    DownscaleParams downscaleConfig_{};

    ContourPointsItem* outerContour_{nullptr};
    ContourPointsItem* innerContour_{nullptr};

    OverlayTextItem* infoOverlay_{nullptr};

    OverlayTextItem* cursorOverlayItem_{nullptr};
    HudOverlayController* cursorOverlayController_{nullptr};

    OverlayTextItem* notificationOverlayItem_{nullptr};
    HudOverlayController* notificationOverlayController_{nullptr};

    MiniMapWidget* miniMap_{nullptr};
    QImage thumbnail_;

    ImageViewerInteraction* interaction_{nullptr};
    ImageViewerListener* listener_{nullptr};

    // --- Throttling ---
    UiFrame pendingFrame_;
    bool hasPendingFrame_{false};

    QElapsedTimer displayTimer_;
    int minDisplayIntervalMs_{0};

    QTimer* throttleTimer_{nullptr};

    bool autoFitEnabled_{true};

    // --- Zoom / Pan ---
    const double minZoom_{0.1};
    const double maxZoom_{100.0};

    QGraphicsBlurEffect* blur_{nullptr};
    bool placeholderVisible_{false};

    bool dragHighlight_{false};

    const bool useEnhancedDisplayConfig_{false};

    bool previousAutoFitEnabled_{true};
    QTransform previousTransform_;
    QPointF previousSceneCenter_;
    int previousHScroll_{0};
    int previousVScroll_{0};
    Qt::ScrollBarPolicy previousHScrollPolicy_{Qt::ScrollBarAsNeeded};
    Qt::ScrollBarPolicy previousVScrollPolicy_{Qt::ScrollBarAsNeeded};

    QBrush previousBackgroundBrush_;

    QTimer inactivityTimer_;
    bool isFullscreen_{false};
    bool isUserActive_{false};
};

} // namespace fluvel
