// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QColor>
#include <QString>
#include <vector>

namespace ofeli_app
{

struct AlgoMetric
{
    QString label;
    QString value;
    QColor color;
};

struct AlgoStats
{
    QString algoName;
    QString status;

    int iteration = 0;   // méta
    double energy = 0.0; // méta (si tu veux)

    std::vector<AlgoMetric> metrics; // dépendant du modèle
};

} // namespace ofeli_app
