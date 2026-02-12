#ifndef COLOR_SELECTOR_WIDGET_HPP
#define COLOR_SELECTOR_WIDGET_HPP

#include <QWidget>
#include <QColor>

#include "color.hpp"

class QComboBox;
class QPushButton;

namespace ofeli_app
{

class ColorSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    ColorSelectorWidget(QWidget* parent,
                        QColor initialColor = Qt::black);

    QColor color() const;
    void setSelectedColor(const QColor& color);

signals:
    void colorSelected(const QColor& color);

private:

    void onIndexChanged();
    void onCustomClicked();

    void addColorItem(const QColor& color,
                      const QString& name);

    QPixmap drawColorSquare(const QColor& color,
                            int size = 12);

    QComboBox* color_cb_;
    QPushButton* custom_pb_;
};

}

#endif // COLOR_SELECTOR_WIDGET_HPP
