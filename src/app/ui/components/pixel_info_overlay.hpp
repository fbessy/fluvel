// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_view.hpp"

#include <QColor>
#include <QFont>
#include <QGraphicsItem>
#include <QPoint>
#include <QPointF>
#include <QString>

namespace ofeli_app
{

class PixelInfoOverlay final : public QGraphicsItem
{
public:
    explicit PixelInfoOverlay(QGraphicsScene* scene);
    ~PixelInfoOverlay() override = default;

    // Mise à jour du contenu et de la position
    void updateInfo(const QPoint& pixel, const QRgb& color, bool isGrayImg,
                    const QPointF& anchorScenePos, ImageView& view);

    void updatePlacement(const QPointF& anchorScenePos, ImageView& view);

    void showOverlay();
    void hideOverlay();

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

private:
    void calc_bounding(const QString& maxStr);

    QString text_;
    QString text_r_;
    QString text_g_;
    QString text_b_;
    QFont font_;
    QRectF boundingRect_;
};

} // namespace ofeli_app
