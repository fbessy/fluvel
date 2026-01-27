#pragma once

#include <QGraphicsItem>
#include <QColor>
#include <QString>
#include <QPointF>
#include <QPoint>
#include <QFont>

namespace ofeli_app {

class PixelInfoOverlay final : public QGraphicsItem
{
public:
    explicit PixelInfoOverlay(QGraphicsScene* scene);
    ~PixelInfoOverlay() override = default;

    // Mise à jour du contenu et de la position
    void updateInfo(const QPoint& pixel,
                    const QRgb& color,
                    bool isGrayImg,
                    const QPointF& displayScenePos);

    void showOverlay();
    void hideOverlay();

    QRectF boundingRect() const override;
    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

private:

    void calc_bounding(const QString& maxStr);

    QString text_;
    QString text_r_;
    QString text_g_;
    QString text_b_;
    QFont font_;
    QRectF  boundingRect_;
};

} // namespace ofeli_app
