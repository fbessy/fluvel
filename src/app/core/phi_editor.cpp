#include "phi_editor.hpp"
#include "application_settings.hpp"

#include <QPainter>

namespace ofeli_app
{

PhiEditor::PhiEditor()
{
    current = AppSettings::instance().initialPhi;
}

void PhiEditor::onImageSizeReady(int width, int height)
{
    if ( !current.isNull() )
    {
        current = current.scaled( width,
                                  height,
                                  Qt::IgnoreAspectRatio,
                                  Qt::FastTransformation);

        emit phiResized(width, height);
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
    emit phiChanged();
}

void PhiEditor::accept()
{
    AppSettings::instance().initialPhi = current;
}

void PhiEditor::reject()
{
    current = AppSettings::instance().initialPhi;
    emit phiChanged();
}

bool PhiEditor::is_redundant(int x, int y)
{
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if ( !(dx == 0 && dy == 0) )
            {
                if ( x+dx >= 0 && x+dx < current.width() &&
                     y+dy >= 0 && y+dy < current.height() )
                {
                    if ( qGray(current.pixel(x,y)) != qGray(current.pixel(x+dx,y+dy)) )
                    {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

} // namespace ofeli_app
