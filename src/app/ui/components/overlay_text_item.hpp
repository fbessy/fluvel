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
};

} // namespace fluvel_app
