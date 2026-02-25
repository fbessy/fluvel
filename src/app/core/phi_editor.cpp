#include "phi_editor.hpp"

#include <QPainter>

namespace ofeli_app
{

PhiEditor::PhiEditor(const QImage& committedPhi)
    : committedPhi_(committedPhi)
{
    editedPhi_ = committedPhi_;
}

void PhiEditor::changeShape(const ShapeInfo& shapeInfo, const QColor& color)
{
    QPainter painter(&editedPhi_);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);

    if (shapeInfo.type == ShapeType::Rectangle)
        painter.drawRect(shapeInfo.boundingBox);
    else if (shapeInfo.type == ShapeType::Ellipse)
        painter.drawEllipse(shapeInfo.boundingBox);
}

void PhiEditor::addShape(const ShapeInfo& shapeInfo)
{
    changeShape(shapeInfo, Qt::white);
    emit editedPhiChanged();
}

void PhiEditor::subtractShape(const ShapeInfo& shapeInfo)
{
    changeShape(shapeInfo, Qt::black);
    emit editedPhiChanged();
}

void PhiEditor::clear()
{
    editedPhi_.fill(Qt::black);
    emit editedPhiCleared();
}

void PhiEditor::accept()
{
    committedPhi_ = editedPhi_;

    emit phiAccepted(committedPhi_);
}

void PhiEditor::reject()
{
    editedPhi_ = committedPhi_;

    emit editedPhiChanged();
}

void PhiEditor::setSize(const QSize& size)
{
    if (editedPhi_.isNull())
        return;

    if (committedPhi_.isNull())
        return;

    if (size != editedPhi_.size())
    {
        editedPhi_ = editedPhi_.scaled(size, Qt::IgnoreAspectRatio, Qt::FastTransformation);

        committedPhi_ = committedPhi_.scaled(size, Qt::IgnoreAspectRatio, Qt::FastTransformation);

        emit phiAccepted(committedPhi_);
    }
}

} // namespace ofeli_app
