#include "pixel_info_overlay.hpp"

#include <QGraphicsScene>
#include <QPainter>
#include <QFontMetrics>

namespace ofeli_app {

static const QString kMaxText =
    "(0000, 0000)\n R:888  G:888  B:888";

static const QString kMaxGrayText =
    "(0000, 0000)\n Gray:888";

PixelInfoOverlay::PixelInfoOverlay(QGraphicsScene* scene)
{
    Q_ASSERT(scene);

    scene->addItem(this);

    setZValue(10'000);          // toujours au-dessus
    setVisible(false);
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

    QFont font; // police par défaut
    font.setStyleHint(QFont::SansSerif);

    // active les chiffres tabulaires
    font.setFeature("tnum", 1);

    font_ = font;
}

void PixelInfoOverlay::calc_bounding(const QString& maxStr)
{
    QFontMetrics fm(QFont{});

    QRect textRect = fm.boundingRect(
        QRect(0, 0, 1000, 1000),
        Qt::AlignLeft | Qt::TextWordWrap,
        maxStr
        );

    boundingRect_ = QRectF(
        0, 0,
        textRect.width()  + 16,
        textRect.height() + 12
        );
}

void PixelInfoOverlay::updateInfo(const QPoint& pixel,
                                  const QRgb& color,
                                  bool isGrayImg,
                                  const QPointF& anchorScenePos,
                                  ImageView& view)
{
    if ( isGrayImg )
    {
        calc_bounding( kMaxGrayText );

        text_ = QString("(%1, %2)\n Gray:%3")
            .arg(pixel.x())
            .arg(pixel.y())
            .arg(qRed(color));
    }
    else
    {
        calc_bounding( kMaxText );

        text_ = QString("(%1, %2)\n R:%3  G:%4  B:%5")
        .arg(pixel.x())
            .arg(pixel.y())
            .arg(qRed(color))
            .arg(qGreen(color))
            .arg(qBlue(color));
    }

    updatePlacement(anchorScenePos, view);
}

void PixelInfoOverlay::updatePlacement(const QPointF& anchorScenePos,
                                       ImageView& view)
{
    constexpr int margin = 8;

    const QSizeF overlaySize = boundingRect().size();
    const QRect viewRect = view.viewport()->rect();

    QPoint anchorViewPos = view.mapFromScene(anchorScenePos);

    QPoint pos = anchorViewPos + QPoint(margin, margin);
    QRectF rect(pos, overlaySize);

    if (rect.right() > viewRect.right())
        pos.setX(anchorViewPos.x() - overlaySize.width() - margin);

    if (rect.bottom() > viewRect.bottom())
        pos.setY(anchorViewPos.y() - overlaySize.height() - margin);

    pos.setX(std::max(pos.x(), viewRect.left()));
    pos.setY(std::max(pos.y(), viewRect.top()));

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

void PixelInfoOverlay::paint(QPainter* painter,
                             const QStyleOptionGraphicsItem*,
                             QWidget*)
{
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setFont(font_);

    // Fond semi-transparent
    painter->setBrush(QColor(0, 0, 0, 180));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(boundingRect_, 5, 5);

    // Texte
    painter->setPen(Qt::white);
    painter->drawText(
        boundingRect_.adjusted(8, 6, -8, -6),
        Qt::AlignCenter | Qt::AlignVCenter,
        text_
        );
}

} // namespace ofeli_app
