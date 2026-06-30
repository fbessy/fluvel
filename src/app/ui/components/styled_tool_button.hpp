// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "ui_appearance.hpp"

#include <QPointer>
#include <QPropertyAnimation>
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
    Q_PROPERTY(qreal transitionProgress READ transitionProgress WRITE setTransitionProgress)

    /**
     * @brief Direction of the icon flip animation.
     */
    enum class FlipDirection
    {
        /// Alternate the flip direction automatically.
        Auto,

        /// Always flip to the left.
        Left,

        /// Always flip to the right.
        Right
    };

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
    void setAnimatedIcon(const QIcon& icon, FlipDirection direction = FlipDirection::Auto);

protected:
    /**
     * @brief Draws the styled button.
     *
     * The button itself is rendered using the current Qt style,
     * while the icon is drawn manually to support animated
     * transitions between icons.
     */
    void paintEvent(QPaintEvent*) override;

private:
    void updateStyle();
    qreal transitionProgress() const;
    void setTransitionProgress(qreal progress);

    /**
     * @brief Updates the direction of the next flip animation.
     *
     * @param direction Requested flip direction.
     */
    void updateTransitionDirection(FlipDirection direction);

    ui::Appearance appearance_;

    QIcon currentIcon_;
    QIcon nextIcon_;

    qreal transitionProgress_{0.0};
    qreal transitionDirection_{-1.0};
    QPointer<QPropertyAnimation> transitionAnimation_;
};

} // namespace fluvel