// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QObject>
#include <QSlider>

class QMouseEvent;
class QWidget;

namespace fluvel
{

/**
 * @brief Slider used to navigate within a media timeline.
 *
 * This slider allows direct seeking by clicking or dragging at any position
 * on the timeline, updating its value according to the mouse position.
 */
class TimelineSlider : public QSlider
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a timeline slider.
     *
     * @param parent Parent widget.
     */
    explicit TimelineSlider(QWidget* parent = nullptr);

protected:
    /**
     * @brief Handles mouse press events.
     *
     * Updates the slider position according to the clicked location.
     *
     * @param event Mouse event.
     */
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * @brief Handles mouse press events.
     *
     * Updates the slider position according to the clicked location.
     *
     * @param event Mouse event.
     */
    void mouseMoveEvent(QMouseEvent* event) override;
};

} // namespace fluvel