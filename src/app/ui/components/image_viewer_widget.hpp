// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "common_settings.hpp"
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

class ImageViewerWidget : public QGraphicsView
{
    Q_OBJECT

public:
    // --- Public API (external users) ---

    // init
    explicit ImageViewerWidget(QWidget* parent = nullptr);

    explicit ImageViewerWidget(const DisplayConfig& displayConfig, const DownscaleConfig& downscaleConfig,
                       QWidget* parent = nullptr);

    ~ImageViewerWidget() override;

    void setInteraction(ImageViewerInteraction* interaction);
    void setListener(ImageViewerListener* listener);

    // Throttling : fps max (0 = désactivé)
    void setMaxDisplayFps(double fps);

    // config
    void applyDisplayConfig(const fluvel_app::DisplayConfig& display);
    void applyDownscaleConfig(const fluvel_app::DownscaleConfig& downscale);

    // display
    void setImage(const QImage& img);
    void setContour(const QVector<QPointF>& outerContour, const QVector<QPointF>& innerContour);
    void setImageAndContour(const UiFrame& uiFrame);
    void clearContour();
    void setText(const QString& text);
    void showPlaceholder(bool showEffect);

    // getters
    const QImage& image() const;
    QImage renderToImage() const;

    // --- Internal API (used by interactions/behaviors) ---

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

signals:
    void imageClicked(int x, int y);
    void frameDisplayed(const fluvel_app::FrameTimestamps& ts);
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
    DownscaleConfig downscaleConfig_;

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
