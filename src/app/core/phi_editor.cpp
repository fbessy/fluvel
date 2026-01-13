#include "phi_editor.hpp"
#include "application_settings.hpp"

#include <QPainter>

namespace ofeli_app
{

PhiEditor::PhiEditor()
{
}

void PhiEditor::changeShape(const ShapeInfo& shapeInfo,
                            const QColor& color)
{
    QPainter painter(&current);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);

    if ( shapeInfo.type == ShapeType::Rectangle )
        painter.drawRect(shapeInfo.boundingBox);
    else if ( shapeInfo.type == ShapeType::Ellipse )
    {
        painter.drawEllipse(shapeInfo.boundingBox);
    }

    emit phiChanged();
}

void PhiEditor::addShape(const ShapeInfo& shapeInfo)
{
    changeShape(shapeInfo, Qt::white);
}

void PhiEditor::subtractShape(const ShapeInfo& shapeInfo)
{
    changeShape(shapeInfo, Qt::black);
}

void PhiEditor::accept()
{
    AppSettings::instance().initialPhi = current;
}

void PhiEditor::clear()
{
    current = AppSettings::instance().initialPhi;
    emit phiChanged();
}

} // namespace ofeli_app
