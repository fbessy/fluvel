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

    [[nodiscard]]
    bool isModernStyle() const;

protected:
    void paintEvent(QPaintEvent* event) override;

    /**
     * @brief Draw additional content after the slider.
     *
     * TimelineSlider uses this to draw the preview time bubble.
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
    virtual QColor progressColor() const;
    virtual QColor handleColor() const;
    virtual QColor handleBorderColor() const;

    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

    virtual QString hoverText(double ratio) const
    {
        Q_UNUSED(ratio);
        return {};
    }

    virtual bool hasHoverBubble() const
    {
        return false;
    }

    void paintHoverBubble(QPainter& painter);

    bool hover_{false};

    bool showHoverBubble_{false};

    QPointF hoverPosition_;

    QString hoverText_;

private:
    ui::Appearance appearance_{ui::Appearance::Modern};
};

} // namespace fluvel