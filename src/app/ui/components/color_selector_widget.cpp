// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "color_selector_widget.hpp"

#include <QColorDialog>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QString>

#include "icon_loader.hpp"

namespace fluvel
{

ColorSelectorWidget::ColorSelectorWidget(QWidget* parent, QColor initialColor)
    : QWidget(parent)
{
    color_cb_ = new QComboBox;

    addColorItem(Qt::red, tr("Red"));
    addColorItem(Qt::green, tr("Green"));
    addColorItem(Qt::blue, tr("Blue"));
    addColorItem(Qt::cyan, tr("Cyan"));
    addColorItem(Qt::magenta, tr("Magenta"));
    addColorItem(Qt::yellow, tr("Yellow"));
    addColorItem(Qt::black, tr("Black"));
    addColorItem(Qt::white, tr("White"));
    addColorItem(Qt::transparent, tr("Custom"));

    QIcon pickerIcon =
        il::loadIcon("color-picker-symbolic", ":/icons/actions/color-picker-symbolic.svg");

    custom_pb_ = new QPushButton;
    custom_pb_->setIcon(pickerIcon);

    auto* layout = new QHBoxLayout(this);
    layout->addWidget(color_cb_);
    layout->addWidget(custom_pb_);

    setSelectedColor(initialColor);

    connect(color_cb_, &QComboBox::currentIndexChanged, this, &ColorSelectorWidget::onIndexChanged);

    connect(custom_pb_, &QPushButton::clicked, this, &ColorSelectorWidget::onCustomClicked);
}

void ColorSelectorWidget::addColorItem(const QColor& color, const QString& name)
{
    color_cb_->addItem(il::createSquare(color), name, color);
}

QColor ColorSelectorWidget::color() const
{
    return color_cb_->currentData().value<QColor>();
}

void ColorSelectorWidget::onIndexChanged()
{
    emit colorSelected(color());
}

void ColorSelectorWidget::onCustomClicked()
{
    QColor chosen = QColorDialog::getColor(color(), this, tr("Custom color selection"));

    if (!chosen.isValid())
        return;

    const int customIndex = color_cb_->findText(tr("Custom"));
    if (customIndex < 0)
        return;

    QSignalBlocker blocker(color_cb_);

    color_cb_->setItemIcon(customIndex, il::createSquare(chosen));
    color_cb_->setItemData(customIndex, chosen);
    color_cb_->setCurrentIndex(customIndex);

    emit colorSelected(chosen);
}

void ColorSelectorWidget::setSelectedColor(const QColor& color)
{
    QSignalBlocker blocker(color_cb_);

    // chercher si la couleur existe déjà
    for (int i = 0; i < color_cb_->count(); ++i)
    {
        const QVariant v = color_cb_->itemData(i);
        if (v.canConvert<QColor>() && v.value<QColor>() == color)
        {
            color_cb_->setCurrentIndex(i);

            emit colorSelected(color);
            return;
        }
    }

    // sinon → custom
    const int customIndex = color_cb_->findText(tr("Custom"));
    if (customIndex >= 0)
    {
        color_cb_->setItemIcon(customIndex, il::createSquare(color));
        color_cb_->setItemData(customIndex, color);
        color_cb_->setCurrentIndex(customIndex);

        emit colorSelected(color);
    }
}

} // namespace fluvel
