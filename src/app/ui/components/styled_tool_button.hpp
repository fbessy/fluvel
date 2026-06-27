// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

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

private:
    void updateStyle();

    ui::Appearance appearance_;
};

} // namespace fluvel