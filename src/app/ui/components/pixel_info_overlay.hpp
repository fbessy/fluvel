// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_widget.hpp"

#include <QtGui/qrgb.h>
#include <QFont>
#include <QGraphicsItem>
#include <QPoint>
#include <QPointF>
#include <QString>

namespace fluvel_app
{

/**
 * @brief Overlay displaying pixel information in the image viewer.
 *
 * This graphics item shows the coordinates and color values of a pixel
 * under the cursor. It is typically anchored near the cursor position
 * and updated dynamically during interaction.
 *
 * The overlay supports both grayscale and RGB images and adapts its
 * displayed content accordingly.
 *
 * Positioning is handled relative to the scene and adjusted using the
 * associated ImageViewerWidget to ensure proper placement in the view.
 *
 * @note The item is added to a QGraphicsScene but is not owned by it.
 */
class PixelInfoOverlay final : public QGraphicsItem
{
public:
    /**
     * @brief Constructs the overlay and attaches it to a scene.
     *      * @param scene Target graphics scene.
     */
    explicit PixelInfoOverlay(QGraphicsScene* scene);

    ~PixelInfoOverlay() override = default;

    /**
     * @brief Updates overlay content and position.
     *      * Sets the displayed pixel coordinates and color values, then updates
     * the overlay placement relative to the given anchor position.
     *      * @param pixel Pixel coordinates in image space.
     * @param color Pixel color value.
     * @param isGrayImg True if the image is grayscale.
     * @param anchorScenePos Anchor position in scene coordinates.
     * @param view Associated image viewer (used for placement).
     */
    void updateInfo(const QPoint& pixel, const QRgb& color, bool isGrayImg,
                    const QPointF& anchorScenePos, ImageViewerWidget& view);

    /**
     * @brief Updates overlay position without changing its content.
     *      * Repositions the overlay relative to the given anchor point,
     * typically following cursor movement.
     *      * @param anchorScenePos Anchor position in scene coordinates.
     * @param view Associated image viewer.
     */
    void updatePlacement(const QPointF& anchorScenePos, ImageViewerWidget& view);

    /// Shows the overlay.
    void showOverlay();

    /// Hides the overlay.
    void hideOverlay();

    /**
     * @brief Returns the bounding rectangle of the overlay.
     */
    QRectF boundingRect() const override;

    /**
     * @brief Paints the overlay content.
     *      * Renders the text containing pixel coordinates and color values.
     */
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

private:
    /**
     * @brief Computes the bounding rectangle based on the text content.
     *      * @param maxStr Reference string used to estimate maximum size.
     */
    void calc_bounding(const QString& maxStr);

    /// Full text displayed (coordinates + values).
    QString text_;

    /// Individual channel strings (RGB or grayscale).
    QString text_r_;
    QString text_g_;
    QString text_b_;

    /// Font used for rendering.
    QFont font_;

    /// Cached bounding rectangle.
    QRectF boundingRect_;
};

} // namespace fluvel_app
