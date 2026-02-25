#ifndef OVERLAY_TEXT_ITEM_HPP
#define OVERLAY_TEXT_ITEM_HPP

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

#endif // OVERLAY_TEXT_ITEM_HPP
