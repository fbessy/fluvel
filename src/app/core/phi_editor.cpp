#include "phi_editor.hpp"
#include "application_settings.hpp"

#include <QPainter>

namespace ofeli_app
{

PhiEditor::PhiEditor()
{
    current = AppSettings::instance().imgSessSettings.initial_phi;
}

void PhiEditor::onImageSizeReady(int width, int height)
{
    if ( !current.isNull() )
    {
        current = current.scaled( width,
                                  height,
                                  Qt::IgnoreAspectRatio,
                                  Qt::FastTransformation);

        emit phiResized( current.width(), current.height() );
    }
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
}

void PhiEditor::addShape(const ShapeInfo& shapeInfo)
{
    changeShape(shapeInfo, Qt::white);
    emit phiChanged();
}

void PhiEditor::subtractShape(const ShapeInfo& shapeInfo)
{
    changeShape(shapeInfo, Qt::black);
    emit phiChanged();
}

void PhiEditor::clear()
{
    current.fill(Qt::black);
    emit phiCleared();
}

void PhiEditor::accept()
{
    AppSettings::instance().imgSessSettings.initial_phi = current;
}

void PhiEditor::reject()
{
    if ( current != AppSettings::instance().imgSessSettings.initial_phi )
    {
        current = AppSettings::instance().imgSessSettings.initial_phi;
        emit phiChanged();
    }
}

} // namespace ofeli_app
