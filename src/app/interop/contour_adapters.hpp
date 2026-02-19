#ifndef CONTOUR_ADAPTERS_HPP
#define CONTOUR_ADAPTERS_HPP

#include "contour_data.hpp"
#include <QVector>
#include <QPoint>

[[nodiscard]] inline QVector<QPoint>
convertToQVector(const ofeli_ip::ExportedContour& contour)
{
    QVector<QPoint> q_contour;
    q_contour.reserve( static_cast<qsizetype>(contour.size()) );

    for ( const auto& point : contour )
    {
        q_contour.emplace_back( point.x,
                                point.y );
    }

    return q_contour;
}

#endif // CONTOUR_ADAPTERS_HPP
