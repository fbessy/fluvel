// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QColor>
#include <QWidget>

class QComboBox;
class QPushButton;

namespace fluvel_app
{

class ColorSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    ColorSelectorWidget(QWidget* parent, QColor initialColor = Qt::black);

    QColor color() const;
    void setSelectedColor(const QColor& color);

signals:
    void colorSelected(const QColor& color);

private:
    void onIndexChanged();
    void onCustomClicked();

    void addColorItem(const QColor& color, const QString& name);

    QPixmap drawColorSquare(const QColor& color, int size = 12);

    QComboBox* color_cb_;
    QPushButton* custom_pb_;
};

} // namespace fluvel_app
