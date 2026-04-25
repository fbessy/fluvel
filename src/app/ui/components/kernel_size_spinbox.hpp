// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QSpinBox>

namespace fluvel_app
{

/**
 * @brief Spin box dedicated to kernel size input.
 *
 * This widget enforces constraints specific to kernel sizes
 * (typically positive odd values) and provides custom validation.
 *
 * It also adapts its suffix dynamically and improves user interaction
 * by handling focus and intermediate input states.
 */
class KernelSizeSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the spin box.
     *      * @param parent Optional parent widget.
     */
    explicit KernelSizeSpinBox(QWidget* parent = nullptr);

protected:
    /**
     * @brief Validates user input.
     *      * Applies custom validation rules to ensure valid kernel sizes
     * and preserves the last acceptable state.
     *      * @param text Current text being edited.
     * @param pos Cursor position.
     * @return Validation state.
     */
    QValidator::State validate(QString& text, int& pos) const override;

    /**
     * @brief Handles focus-in events.
     *      * Used to adjust display or restore a valid state when the widget
     * gains focus.
     *      * @param event Focus event.
     */
    void focusInEvent(QFocusEvent* event) override;

private:
    /**
     * @brief Sets the displayed suffix.
     *      * @param text Suffix text.
     */
    void setSuffix(const QString& text);

    /// Last validation result considered acceptable.
    QValidator::State previousResult_{QValidator::Acceptable};
};

} // namespace fluvel_app
