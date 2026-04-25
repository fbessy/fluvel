// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_viewer_widget.hpp"
#include "image_viewer_behavior.hpp"

#include <QObject>
#include <QPoint>

class QMouseEvent;
class ImageViewerWidget;

namespace fluvel_app
{

/**
 * @brief Behavior handling shape initialization interactions.
 *
 * This behavior interprets user input events (mouse and wheel)
 * and emits high-level signals to drive shape creation and editing.
 *
 * It does not perform any processing itself but acts as a bridge
 * between the ImageViewerWidget interaction system and higher-level
 * application logic.
 *
 * Typical interactions:
 * - Mouse move → preview shape position
 * - Mouse press → add/remove shape elements
 * - Mouse wheel → resize shape
 *
 * @note This behavior does not own or modify any shape directly.
 *       All actions are delegated through signals.
 */
class InitializationBehavior : public QObject, public ImageViewerBehavior
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the initialization behavior.
     *      * @param parent Optional QObject parent.
     */
    explicit InitializationBehavior(QObject* parent = nullptr);

    /**
     * @brief Handles mouse move events.
     *      * Emits a preview request at the current image position.
     *      * @param view Associated image viewer.
     * @param e Mouse event.
     * @return false (event is not consumed).
     */
    bool mouseMove(ImageViewerWidget& view, QMouseEvent* e) override;

    /**
     * @brief Handles mouse press events.
     *      * Emits shape modification signals depending on the interaction
     * (e.g. add or remove).
     *      * @param view Associated image viewer.
     * @param e Mouse event.
     * @return false (event is not consumed).
     */
    bool mousePress(ImageViewerWidget& view, QMouseEvent* e) override;

    /**
     * @brief Handles wheel events.
     *      * Emits a resize request for the shape.
     *      * @param view Associated image viewer.
     * @param we Wheel event.
     * @return false (event is not consumed).
     */
    bool wheel(ImageViewerWidget& view, QWheelEvent* we) override;

signals:
    /**
     * @brief Requests a preview of the shape at a given position.
     *      * @param imagePos Position in image coordinates.
     */
    void previewShapeRequested(QPoint imagePos);

    /**
     * @brief Requests adding a shape element at a given position.
     *      * @param imagePos Position in image coordinates.
     */
    void addShapeRequested(QPoint imagePos);

    /**
     * @brief Requests removing a shape element at a given position.
     *      * @param imagePos Position in image coordinates.
     */
    void removeShapeRequested(QPoint imagePos);

    /**
     * @brief Requests resizing the current shape.
     *      * @param delta Wheel delta or size increment.
     */
    void resizeShapeRequested(int delta);

    /**
     * @brief Requests toggling shape state or mode.
     */
    void toggleShapeRequested();
};

} // namespace fluvel_app
