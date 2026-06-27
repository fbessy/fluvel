#pragma once

#include "styled_slider.hpp"

class QMouseEvent;

namespace fluvel
{

/**
 * @brief Slider used to navigate within a media timeline.
 *
 * TimelineSlider extends StyledSlider by displaying the
 * corresponding media time while hovering the slider.
 */
class TimelineSlider : public StyledSlider
{
public:
    explicit TimelineSlider(QWidget* parent = nullptr,
                            ui::Appearance appearance = ui::Appearance::Modern);

protected:
    QString hoverText(double ratio) const override;

    bool hasHoverBubble() const override;
};

} // namespace fluvel