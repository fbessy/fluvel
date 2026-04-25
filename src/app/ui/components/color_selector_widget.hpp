// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QColor>
#include <QWidget>

class QComboBox;
class QPushButton;
class QString;

namespace fluvel_app
{

/**
 * @brief Widget for selecting a color.
 *
 * This widget provides a combo box with predefined colors and an option
 * to choose a custom color.
 *
 * It emits colorSelected() whenever the selected color changes.
 *
 * @note Custom color selection typically opens a QColorDialog.
 */
class ColorSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the color selector widget.
     *      * @param parent Parent widget.
     * @param initialColor Initial selected color.
     */
    ColorSelectorWidget(QWidget* parent, QColor initialColor = Qt::black);

    /**
     * @brief Returns the currently selected color.
     */
    QColor color() const;

    /**
     * @brief Sets the selected color.
     *      * @param color Color to select.
     */
    void setSelectedColor(const QColor& color);

signals:
    /**
     * @brief Emitted when a color is selected.
     *      * @param color Selected color.
     */
    void colorSelected(const QColor& color);

private:
    /// Handles combo box selection changes.
    void onIndexChanged();

    /// Opens custom color selection.
    void onCustomClicked();

    void addColorItem(const QColor& color, const QString& name);

    QPixmap drawColorSquare(const QColor& color, int size = 12);

    QComboBox* color_cb_;
    QPushButton* custom_pb_;
};

} // namespace fluvel_app
