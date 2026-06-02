// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include <QImage>
#include <QMouseEvent>
#include <QPixmap>
#include <QRectF>
#include <QWidget>

namespace fluvel
{

/**
 * @brief Interactive overview of the currently displayed image.
 *
 * The mini-map displays a reduced representation of the image together
 * with the currently visible viewport. It allows users to quickly
 * navigate large images by clicking or dragging inside the mini-map.
 *
 * The widget emits centerRequested() whenever the user selects a new
 * position to center in the main image view.
 */
class MiniMapWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs an empty mini-map widget.
     *
     * @param parent Optional parent widget.
     */
    explicit MiniMapWidget(QWidget* parent = nullptr);

    /**
     * @brief Updates the image displayed in the mini-map.
     *
     * The image is internally converted to a thumbnail representation
     * suitable for display inside the widget.
     *
     * @param image Source image.
     */
    void setImage(const QImage& image);

    /**
     * @brief Sets the bounds of the scene represented by the mini-map.
     *
     * @param sceneRect Scene rectangle in scene coordinates.
     */
    void setSceneRect(const QRectF& sceneRect);

    /**
     * @brief Sets the currently visible viewport rectangle.
     *
     * This rectangle is displayed as an overlay on top of the mini-map.
     *
     * @param visibleRect Visible area in scene coordinates.
     */
    void setVisibleRect(const QRectF& visibleRect);

signals:
    /**
     * @brief Requests the main view to center on a scene position.
     *
     * Emitted when the user clicks or drags inside the mini-map.
     *
     * @param scenePosition Target position in scene coordinates.
     */
    void centerRequested(const QPointF& scenePosition);

protected:
    /**
     * @brief Paints the mini-map contents.
     */
    void paintEvent(QPaintEvent* event) override;

    /**
     * @brief Handles mouse press navigation.
     */
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * @brief Handles mouse drag navigation.
     */
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    /**
     * @brief Converts a mini-map position into scene coordinates.
     *
     * @param position Position inside the mini-map widget.
     * @return Corresponding scene position.
     */
    QPointF scenePositionFromMiniMap(const QPointF& position) const;

    QPixmap thumbnail_;

    QRectF sceneRect_;
    QRectF visibleRect_;
};

} // namespace fluvel