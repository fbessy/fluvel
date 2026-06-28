#pragma once

#include "styled_slider.hpp"

#include <QString>

class QWidget;

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
    /**
     * @brief Constructs a timeline slider.
     *
     * @param parent Parent widget.
     * @param appearance Slider appearance.
     */
    explicit TimelineSlider(QWidget* parent = nullptr,
                            ui::Appearance appearance = ui::Appearance::Modern);

protected:
    /**
     * @brief Returns the formatted media time corresponding to the hovered position.
     *
     * @param ratio Normalized slider position in the range [0,1].
     * @return Formatted time string.
     */
    QString hoverText(double ratio) const override;
};

} // namespace fluvel