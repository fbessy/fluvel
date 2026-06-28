// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "jump_slider.hpp"
#include "ui_appearance.hpp"

#include <QColor>
#include <QPainter>

/**
 * @brief Base class for Fluvel custom sliders.
 *
 * StyledSlider provides the modern slider appearance used by Fluvel.
 * It draws a rounded groove, a colored progress bar and a circular handle.
 *
 * Derived classes can optionally draw additional overlays
 * (timeline preview, labels, etc.) by overriding paintOverlay().
 *
 * When the Native style is selected, the widget falls back to the
 * standard Qt QSlider rendering.
 */
namespace fluvel
{

class StyledSlider : public JumpSlider
{
    Q_OBJECT

public:
    explicit StyledSlider(QWidget* parent = nullptr,
                          ui::Appearance appearance = ui::Appearance::Modern);

    /**
     * @brief Returns whether the slider uses the custom Fluvel appearance.
     *
     * @return True when the Modern appearance is enabled, false when
     * the native Qt style is used.
     */
    [[nodiscard]]
    bool isModernStyle() const;

protected:
    void paintEvent(QPaintEvent* event) override;

    /**
     * @brief Draws additional content on top of the slider.
     *
     * Called after the groove, progress bar and handle have been painted.
     * Derived classes can use this to render custom decorations such as
     * timeline markers or buffered ranges.
     */
    virtual void paintOverlay(QPainter&)
    {
    }

    QRect grooveRect() const;

    QRect progressRect() const;

    QPoint handleCenter() const;

    [[nodiscard]]
    double ratio() const;

    virtual QColor grooveColor() const;
    virtual int grooveHeight() const;
    virtual int grooveHoverHeight() const;
    virtual QColor progressColor() const;

    virtual QColor handleColor() const;
    virtual QColor handleBorderColor() const;
    virtual int handleRadius() const;
    virtual int handleHoverRadius() const;

    virtual int sliderHeight() const;
    virtual int topMargin() const;
    virtual int bottomMargin() const;

    virtual int hoverFontPointSize() const;
    virtual QMargins hoverBubbleMargins() const;
    virtual int hoverBubbleRadius() const;
    virtual int hoverBubbleOffset() const;

    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

    /**
     * @brief Returns the text displayed while hovering the slider.
     *
     * @param ratio Slider position normalized in the range [0,1].
     * @return Text displayed in the hover bubble or tooltip.
     */
    virtual QString hoverText(double ratio) const
    {
        Q_UNUSED(ratio);
        return {};
    }

    /**
     * @brief Paints the hover bubble in Modern appearance.
     *
     * Native appearance uses a standard QToolTip instead.
     */
    void paintHoverBubble(QPainter& painter);

    /// True while the mouse is hovering the slider.
    bool hover_{false};

    /// Indicates whether the custom hover bubble should be painted.
    bool showHoverBubble_{false};

    /// Mouse position used to place the hover bubble.
    QPointF hoverPosition_;

    /// Text displayed in the hover bubble or native tooltip.
    QString bubbleText_;

private:
    ui::Appearance appearance_{ui::Appearance::Modern};
};

} // namespace fluvel