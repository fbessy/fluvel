// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "pixel_info_overlay.hpp"

#include <QFontMetrics>
#include <QGraphicsScene>
#include <QObject>
#include <QPainter>

namespace fluvel
{

QString maxText()
{
    return QObject::tr("(0000, 0000)\nR: 888  G: 888  B: 888");
}

QString maxGrayText()
{
    return QObject::tr("(0000, 0000)\nGray: 888");
}

PixelInfoOverlay::PixelInfoOverlay(QGraphicsScene* scene)
{
    Q_ASSERT(scene);

    setAcceptedMouseButtons(Qt::NoButton);
    setAcceptHoverEvents(false);

    scene->addItem(this);

    setZValue(10'000); // toujours au-dessus
    setVisible(false);
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

    QFont font;
    font.setStyleHint(QFont::SansSerif);
    font.setPointSize(13);
    font.setBold(true);

    // chiffres tabulaires
    font.setFeature("tnum", 1);

    font_ = font;
}

void PixelInfoOverlay::calcBounding(const QString& maxStr)
{
    QFontMetrics fm(font_);

    QRect textRect =
        fm.boundingRect(QRect(0, 0, 1000, 1000), Qt::AlignLeft | Qt::TextWordWrap, maxStr);

    boundingRect_ = QRectF(0, 0, textRect.width() + 20, textRect.height() + 16);
}

void PixelInfoOverlay::updateInfo(const QPoint& pixel, const QRgb& color, bool isGrayImg,
                                  const QPointF& anchorScenePos, ImageViewerWidget& view)
{
    if (isGrayImg)
    {
        calcBounding(maxGrayText());

        text_ = QObject::tr("(%1, %2)\nGray: %3").arg(pixel.x()).arg(pixel.y()).arg(qRed(color));
    }
    else
    {
        calcBounding(maxText());

        text_ = QObject::tr("(%1, %2)\nR: %3  G: %4  B: %5")
                    .arg(pixel.x())
                    .arg(pixel.y())
                    .arg(qRed(color))
                    .arg(qGreen(color))
                    .arg(qBlue(color));
    }

    updatePlacement(anchorScenePos, view);
}

void PixelInfoOverlay::updatePlacement(const QPointF& anchorScenePos, ImageViewerWidget& view)
{
    constexpr int margin = 8;

    const QPoint anchorViewPos = view.mapFromScene(anchorScenePos);

    const QSize overlaySize{static_cast<int>(std::ceil(boundingRect().width())),
                            static_cast<int>(std::ceil(boundingRect().height()))};

    const QPoint pos =
        view.adjustOverlayPosition(anchorViewPos, overlaySize, QPoint(margin, margin));

    setPos(view.mapToScene(pos));
}

void PixelInfoOverlay::showOverlay()
{
    setVisible(true);
}

void PixelInfoOverlay::hideOverlay()
{
    setVisible(false);
}

QRectF PixelInfoOverlay::boundingRect() const
{
    return boundingRect_;
}

void PixelInfoOverlay::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setFont(font_);

    // Fond semi-transparent
    painter->setBrush(QColor(0, 0, 0, 180));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(boundingRect_, 5, 5);

    // Texte
    painter->setPen(Qt::white);
    painter->drawText(boundingRect_.adjusted(10, 8, -10, -8), Qt::AlignCenter | Qt::AlignVCenter,
                      text_);
}

} // namespace fluvel
