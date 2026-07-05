// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QColor>
#include <QFont>
#include <QGraphicsObject>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

namespace fluvel
{

/**
 * @brief Draggable text overlay displayed on top of a QGraphicsView.
 *
 * The overlay automatically adapts its size to the displayed text and remains
 * readable regardless of the current view transformation
 * (ItemIgnoresTransformations).
 *
 * It is used by several overlay controllers (HUD, information overlay, ...)
 * and can be customized through its font, colors, padding and corner radius.
 */
class OverlayTextItem : public QGraphicsObject
{
    Q_OBJECT

    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    /**
     * @brief Constructs an overlay text item.
     * @param parent Optional parent graphics item.
     */
    explicit OverlayTextItem(QGraphicsItem* parent = nullptr);

    /**
     * @brief Returns the current text.
     */
    [[nodiscard]] const QString& text() const
    {
        return text_;
    }

    /**
     * @brief Sets the displayed text.
     *
     * The geometry is automatically updated to fit the new text.
     *
     * @param text Text to display.
     */
    void setText(const QString& text);

    /**
     * @brief Returns the current font.
     */
    [[nodiscard]] const QFont& font() const;

    /**
     * @brief Sets the font used to render the text.
     *
     * The overlay geometry is automatically recomputed.
     *
     * @param font New font.
     */
    void setFont(const QFont& font);

    /**
     * @brief Returns the text alignment.
     */
    [[nodiscard]] Qt::Alignment alignment() const
    {
        return alignment_;
    }

    /**
     * @brief Sets the text alignment.
     * @param alignment Text alignment.
     */
    void setAlignment(Qt::Alignment alignment);

    /**
     * @brief Returns the minimum overlay width.
     */
    [[nodiscard]] qreal minWidth() const
    {
        return minWidth_;
    }

    /**
     * @brief Sets the minimum overlay width.
     *
     * The overlay will never become smaller than this width.
     *
     * @param width Minimum width in pixels.
     */
    void setMinWidth(qreal width);

    /**
     * @brief Returns the text padding.
     */
    [[nodiscard]] int padding() const;

    /**
     * @brief Sets the padding around the text.
     *
     * The overlay geometry is automatically recomputed.
     *
     * @param padding Padding in pixels.
     */
    void setPadding(int padding);

    /**
     * @brief Returns the background corner radius.
     */
    [[nodiscard]] qreal cornerRadius() const;

    /**
     * @brief Sets the background corner radius.
     * @param radius Corner radius in pixels.
     */
    void setCornerRadius(qreal radius);

    /**
     * @brief Returns the background color.
     */
    [[nodiscard]] const QColor& backgroundColor() const;

    /**
     * @brief Sets the background color.
     * @param color Background color.
     */
    void setBackgroundColor(const QColor& color);

    /**
     * @brief Returns the text color.
     */
    [[nodiscard]] const QColor& textColor() const;

    /**
     * @brief Sets the text color.
     * @param color Text color.
     */
    void setTextColor(const QColor& color);

    /**
     * @brief Returns the overlay bounding rectangle.
     */
    [[nodiscard]] QRectF boundingRect() const override;

    /**
     * @brief Paints the overlay.
     */
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    /**
     * @brief Updates the overlay geometry.
     *
     * Recomputes the bounding rectangle according to the current text, font,
     * padding and minimum width, then schedules a repaint.
     */
    void updateGeometry();

    QString text_;

    QFont font_;

    QColor backgroundColor_{0, 0, 0, 180};

    QColor textColor_{Qt::white};

    int padding_{8};

    qreal cornerRadius_{5.0};

    qreal minWidth_{0.0};

    Qt::Alignment alignment_{Qt::AlignCenter};

    QRectF rect_{};
};

} // namespace fluvel