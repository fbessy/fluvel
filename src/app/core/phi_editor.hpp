#ifndef PHI_EDITOR_HPP
#define PHI_EDITOR_HPP

#include <QObject>
#include <vector>
#include <QRect>
#include <QImage>

#include "grid2d.hpp"
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

    PhiEditor(const QImage& committedPhi);

    void addShape(const ShapeInfo& shape);
    void subtractShape(const ShapeInfo& shape);
    void clear();

    void accept();
    void reject();

    void setSize(const QSize& size);

    const QImage& phi() const { return editedPhi_; }

signals:
    void phiAccepted(const QImage& committedPhi);
    void editedPhiChanged();
    void editedPhiCleared();

private:

    void changeShape(const ShapeInfo& shapeInfo,
                     const QColor& color);

    QImage editedPhi_;
    QImage committedPhi_;
};

} // namespace ofeli_app

#endif // PHI_EDITOR_HPP
