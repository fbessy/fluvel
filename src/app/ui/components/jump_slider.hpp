// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QSlider>

class QMouseEvent;

namespace fluvel
{

/**
 * @brief Slider supporting direct positioning.
 *
 * Unlike the default QSlider behavior, clicking on the slider groove
 * immediately moves the handle to the clicked position.
 */
class JumpSlider : public QSlider
{
public:
    /**
     * @brief Constructs a jump slider with the specified orientation.
     *
     * @param parent Parent widget.
     * @param orientation Orientation of the slider.
     */
    explicit JumpSlider(QWidget* parent = nullptr, Qt::Orientation orientation = Qt::Horizontal);

protected:
    /**
     * @brief Handles mouse press events.
     *
     * Updates the slider position according to the clicked location.
     *
     * @param event Mouse event.
     */
    void mousePressEvent(QMouseEvent* event) override;
};

} // namespace fluvel