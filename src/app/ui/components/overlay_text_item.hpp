// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QFont>
#include <QGraphicsObject>
#include <QPainter>

namespace fluvel_app
{

/**
 * @brief Graphics item displaying a text overlay.
 *
 * This item renders a text string on top of the scene, typically used
 * for transient UI elements such as zoom level or status messages.
 *
 * The text is drawn with a configurable font and padded bounding box.
 * The bounding rectangle is updated whenever the text changes.
 *
 * The item can be animated (e.g. opacity) since it inherits from QGraphicsObject.
 */
class OverlayTextItem : public QGraphicsObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the overlay text item.
     *      * @param parent Optional parent graphics item.
     */
    OverlayTextItem(QGraphicsItem* parent = nullptr);

    /**
     * @brief Sets the displayed text.
     *      * Updates the internal text and recomputes the bounding rectangle.
     *      * @param text Text to display.
     */
    void setText(const QString& text);

    /**
     * @brief Sets the text alignment inside the overlay bounding box.
     *      * Controls how the diagnostic text is positioned within the overlay.
     * Typical values include:
     * - Qt::AlignCenter | Qt::AlignVCenter (default, suited for video/HUD display)
     * - Qt::AlignLeft | Qt::AlignTop (suited for debug or image analysis)
     *      * @param align Qt alignment flags (combination of Qt::AlignmentFlag).
     *      * @note This affects only the text layout, not the overlay position itself.
     */
    void setAlignment(Qt::Alignment align);

    /**
     * @brief Sets the minimum width of the overlay background.
     *      * If set to 0, the width adapts to the text content (dynamic layout).
     * If > 0, the width is clamped to at least this value (stable layout).
     */
    void setMinWidth(qreal w);

    /**
     * @brief Returns the bounding rectangle of the item.
     *      * Includes padding around the rendered text.
     */
    QRectF boundingRect() const override;

protected:
    /**
     * @brief Paints the text overlay.
     *      * Renders the text within the bounding rectangle using the configured font.
     */
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

private:
    /// Text currently displayed.
    QString text_;

    /// Font used for rendering.
    QFont font_;

    /// Cached bounding rectangle including padding.
    QRectF rect_;

    /// Padding (in pixels) around the text.
    int padding_{8};

    /**
     * @brief Alignment used to render the overlay text.
     *      * Defines how the text is positioned inside the overlay bounding box.
     * Default is centered (Qt::AlignCenter | Qt::AlignVCenter), suitable for
     * HUD-style overlays (e.g. video or zoom feedback).
     *      * Can be changed to left/top alignment for debug or analysis overlays.
     */
    Qt::Alignment alignment_{Qt::AlignCenter | Qt::AlignVCenter};

    /**
     * @brief Minimum width of the overlay background in pixels.
     *      * If set to 0, the overlay width adapts dynamically to the text content.
     * If greater than 0, the width is clamped to at least this value,
     * ensuring a stable layout (useful for multi-line debug overlays).
     */
    qreal minWidth_{0.0};
};

} // namespace fluvel_app
