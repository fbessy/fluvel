// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "animated_icon.hpp"
#include "ui_appearance.hpp"

#include <QToolButton>

namespace fluvel
{

/**
 * @brief Tool button supporting Fluvel visual styles.
 *
 * The widget provides either the native Qt appearance or the
 * custom Fluvel modern appearance used by fullscreen controls.
 */
class StyledToolButton : public QToolButton
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a styled tool button.
     *
     * @param parent Parent widget.
     * @param appearance Desired appearance.
     */
    explicit StyledToolButton(QWidget* parent = nullptr,
                              ui::Appearance appearance = ui::Appearance::Modern);

    /**
     * @brief Returns the current appearance.
     */
    ui::Appearance appearance() const;

    /**
     * @brief Changes the button appearance.
     */
    void setAppearance(ui::Appearance appearance);

    /**
     * @brief Changes the button icon using an animated flip transition.
     *
     * The current icon smoothly transitions to the new icon using
     * a perspective-like flip animation combined with a cross-fade.
     *
     * @param icon Target icon.
     * @param direction Flip direction.
     */
    void setAnimatedIcon(const QIcon& icon,
                         AnimatedIcon::FlipDirection direction = AnimatedIcon::FlipDirection::Auto);

protected:
    /**
     * @brief Draws the styled button.
     *
     * The button frame is rendered using the current Qt style,
     * while icon rendering is delegated to AnimatedIcon to support
     * animated transitions.
     */
    void paintEvent(QPaintEvent*) override;

private:
    void updateStyle();

    ui::Appearance appearance_;
    AnimatedIcon animatedIcon_;
};

} // namespace fluvel