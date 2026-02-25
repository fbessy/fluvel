// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QFont>
#include <QGraphicsItem>
#include <QPainter>

namespace ofeli_app
{

class OverlayTextItem : public QGraphicsItem
{
public:
    OverlayTextItem(QGraphicsItem* parent = nullptr);

    void setText(const QString& text);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

    QRectF boundingRect() const override;

private:
    QString text_;

    QFont font_;
    QRectF rect_;
    int padding_ = 8;
};

} // namespace ofeli_app
