#pragma once

#include <QString>
#include <QColor>
#include <vector>

namespace ofeli_app
{

struct AlgoMetric
{
    QString label;
    QString value;
    QColor  color;
};

struct AlgoStats
{
    QString algoName;
    QString status;

    int iteration = 0;   // méta
    double energy = 0.0; // méta (si tu veux)

    std::vector<AlgoMetric> metrics; // dépendant du modèle
};

}
