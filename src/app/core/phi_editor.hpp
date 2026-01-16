#ifndef PHI_EDITOR_HPP
#define PHI_EDITOR_HPP

#include <QObject>
#include <vector>
#include <QRect>
#include <QImage>

#include "matrix.hpp"
#include "active_contour.hpp"
#include "shape_type.hpp"

namespace ofeli_app
{

struct ShapeInfo {
    ShapeType type;
    QRect boundingBox;
};

class PhiEditor : public QObject
{
    Q_OBJECT

public:

    PhiEditor();

    void addShape(const ShapeInfo& shape);
    void subtractShape(const ShapeInfo& shape);
    void clear();

    void accept();
    void reject();

    const QImage& phi() const { return current; }
    bool is_redundant(int x, int y);

public slots:
    void onImageSizeReady(int width, int height);

signals:
    void phiChanged();
    void phiCleared();
    void phiResized(int width, int height);

private:

    void changeShape(const ShapeInfo& shapeInfo,
                     const QColor& color);

    QImage current;
};

} // namespace ofeli_app

#endif // PHI_EDITOR_HPP
