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
    AppSettings::instance().initialPhi = current;
}

void PhiEditor::reject()
{
    if ( current != AppSettings::instance().initialPhi )
    {
        current = AppSettings::instance().initialPhi;
        emit phiChanged();
    }
}

bool PhiEditor::is_redundant(int x, int y)
{
    const int w = current.width();
    const int h = current.height();

    const uchar center = current.constScanLine(y)[x];

    for (int dy = -1; dy <= 1; ++dy)
    {
        const int ny = y + dy;
        if (ny < 0 || ny >= h)
            continue;

        const uchar* line = current.constScanLine(ny);

        for (int dx = -1; dx <= 1; ++dx)
        {
            if (dx == 0 && dy == 0)
                continue;

            const int nx = x + dx;
            if (nx < 0 || nx >= w)
                continue;

            if (line[nx] != center)
                return false;
        }
    }

    return true;
}

} // namespace ofeli_app
