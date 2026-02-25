// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include <QPainter>

#include "algo_info_overlay.hpp"

namespace ofeli_app
{

AlgoInfoOverlay::AlgoInfoOverlay(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);

    setFixedSize(260, 90); // ajustable
}

void AlgoInfoOverlay::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // fond
    p.setBrush(QColor(0, 0, 0, 140));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect(), 6, 6);

    QFont font = p.font();
    font.setPointSize(9);
    font.setBold(true);
    p.setFont(font);

    int y = 18;

    auto drawLine = [&](const QColor& color, const QString& text)
    {
        p.setPen(color);
        p.drawText(10, y, text);
        y += 16;
    };

    drawLine(Qt::white, m_algoName_);
    drawLine(QColor(180, 180, 180), QString("Iteration: %1").arg(stats_.iteration));
    drawLine(QColor(100, 220, 100), QString("Energy: %1").arg(m_energy_, 0, 'f', 4));
    drawLine(QColor(200, 200, 120), m_status_);
}

void AlgoInfoOverlay::resizeEvent(QResizeEvent*)
{
    // coin haut-gauche
    move(5, 60);
}

void AlgoInfoOverlay::setIteration(int iter)
{
    m_iteration_ = iter;
    update();
}

void AlgoInfoOverlay::setEnergy(double energy)
{
    m_energy_ = energy;
    update();
}

void AlgoInfoOverlay::setStats(const AlgoStats& stats)
{
    stats_ = stats;
    update();
}

} // namespace ofeli_app
