// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QFont>
#include <QGraphicsObject>
#include <QPainter>

namespace fluvel_app
{

class OverlayTextItem : public QGraphicsObject
{
    Q_OBJECT
public:
    OverlayTextItem(QGraphicsItem* parent = nullptr);

    void setText(const QString& text);

    QRectF boundingRect() const override;

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

private:
    QString text_;

    QFont font_;
    QRectF rect_;
    int padding_{8};
};

} // namespace fluvel_app
