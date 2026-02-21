#ifndef CONTOUR_ADAPTERS_HPP
#define CONTOUR_ADAPTERS_HPP

#include "contour_data.hpp"
#include <QVector>
#include <QPoint>

[[nodiscard]] inline QVector<QPointF>
convertToQVector(const ofeli_ip::ExportedContour& contour)
{
    QVector<QPointF> q_contour;
    q_contour.reserve( static_cast<qsizetype>(contour.size()) );

    for ( const auto& point : contour )
    {
        q_contour.emplace_back( static_cast<qreal>(point.x) + 0.5,
                                static_cast<qreal>(point.y) + 0.5 );
    }

    return q_contour;
}

#endif // CONTOUR_ADAPTERS_HPP
