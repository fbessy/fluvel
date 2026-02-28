// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "common_settings.hpp"
#include "contour_point_item.hpp"
#include "image_view_listener.hpp"
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

namespace ofeli_app
{

class ImageViewInteraction;

class ImageView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageView(QWidget* parent = nullptr);

    explicit ImageView(const DisplayConfig& displayConfig, const DownscaleConfig& downscaleConfig,
                       QWidget* parent = nullptr);

    // Throttling : fps max (0 = désactivé)
    void setMaxDisplayFps(double fps);

    const QImage& image() const;
    QImage renderToImage() const;

    void setInteraction(ImageViewInteraction* interaction);

    double currentZoom() const;
    void scaleView(double sx, double sy);
    void translateView(double dx, double dy);

    void toggleFullscreen();
    void applyAutoFit();
    void userInteracted();

    QPoint imageCoordinatesFromView(const QPoint& viewPos) const;
    QRgb pixelColorAt(const QPoint& imagePos) const;

    void setListener(ImageViewListener* listener);
    void onColorPicked(const QColor& color, const QPoint& imagePos);

    bool hasImage() const;
    bool isPanRelevant() const;
    QGraphicsScene* graphicsScene() const;

    bool isGrayscale() const;

    void notifyImageDropped(const QString& path);
    void setDragHighlight(bool enabled);

public slots:
    void setImage(const QImage& img);

    void setContour(const QVector<QPointF>& l_out, const QVector<QPointF>& l_in);

    void setImageAndContour(const QImage& image, const QVector<QPointF>& l_out,
                            const QVector<QPointF>& l_in, qint64 receiveTs);

    void setText(const QString& text);

    void showPlaceholder(bool showEffect);

    void applyDisplayConfig(const ofeli_app::DisplayConfig& display);
    void applyDownscaleConfig(const ofeli_app::DownscaleConfig& downscale);
    void updateFlip();
    void updateSmoothDisplay();
    void updateTextOverlayVisibility();
    void clearOverlays();

protected:
    void wheelEvent(QWheelEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void drawForeground(QPainter* painter, const QRectF& rect) override;

    void enterEvent(QEnterEvent*) override;

    void resizeEvent(QResizeEvent* event) override;

private slots:
    void flushPendingFrame();

private:
    void initialize();

    void updatePixmap(const QImage& img);
    double getCurrentZoom() const;

    QPoint textPosition() const;
    void setTextPosition(QPoint position);

    void updateContourColors();
    void upscaleItems();
    void updateDisplayWithConfig();

    QColor desaturateAndDarken(const QColor& original, qreal saturationFactor, qreal valueFactor);

    QImage darkenImage(const QImage& image);

    QGraphicsScene* scene_ = nullptr;
    QGraphicsItemGroup* contentRoot_ = nullptr;
    QGraphicsPixmapItem* pixmapItem_ = nullptr;

    bool autoFitEnabled_ = true;

    // --- Zoom / Pan ---
    const double minZoom_ = 0.1;
    const double maxZoom_ = 20.0;

    // --- Fullscreen ---
    bool isFullScreenMode_ = false;
    QRect normalGeometry_;
    Qt::WindowFlags normalWindowFlags_;

    // --- Throttling ---
    QImage pendingFrame_;
    bool hasPendingFrame_ = false;

    QElapsedTimer displayTimer_;
    int minDisplayIntervalMs_ = 0;

    QTimer* throttleTimer_ = nullptr;

    QImage lastDisplayedImage_;

    qint64 lastReceiveTs_;

    ImageViewInteraction* m_interaction_ = nullptr;
    ImageViewListener* listener_ = nullptr;

    DisplayConfig displayConfig_;
    DownscaleConfig downscaleConfig_;

    ContourPointsItem* l_out_ = nullptr;
    ContourPointsItem* l_in_ = nullptr;

    OverlayTextItem* overlay_ = nullptr;

    bool paused_ = false;
    QGraphicsBlurEffect* blur_ = nullptr;

    bool dragHighlight_ = false;

signals:
    void imageClicked(int x, int y);
    void frameDisplayed(qint64 receiveTs, qint64 displayTs);
    void imageDropped(const QString& path);
};

} // namespace ofeli_app
