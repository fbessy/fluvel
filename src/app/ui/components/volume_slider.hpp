// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "styled_slider.hpp"
#include "ui_appearance.hpp"

class QMouseEvent;

namespace fluvel
{

/**
 * @brief Slider used to control audio volume.
 *
 * This slider supports direct positioning by clicking anywhere on the
 * slider groove and displays the corresponding volume percentage while
 * hovering over the control.
 */
class VolumeSlider : public StyledSlider
{
public:
    /**
     * @brief Constructs a volume slider.
     *
     * @param parent Parent widget.
     */
    explicit VolumeSlider(QWidget* parent = nullptr,
                          ui::Appearance appearance = ui::Appearance::Modern);

protected:
    QString hoverText(double ratio) const override;

    bool hasHoverBubble() const override;
};

} // namespace fluvel