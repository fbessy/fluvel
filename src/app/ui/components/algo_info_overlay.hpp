#ifndef ALGO_INFO_OVERLAY_HPP
#define ALGO_INFO_OVERLAY_HPP

#include <QWidget>
#include <QString>

#include "algo_stats.hpp"

namespace ofeli_app
{

class AlgoInfoOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit AlgoInfoOverlay(QWidget* parent = nullptr);

    void setAlgoName(const QString& name);
    void setIteration(int iter);
    void setEnergy(double energy);
    void setStatus(const QString& status);
    void setStats(const AlgoStats& stats);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent*) override;

private:
    QString m_algoName_;
    QString m_status_;
    int     m_iteration_ = 0;
    double  m_energy_    = 0.0;
    AlgoStats stats_;
};

}

#endif // ALGO_INFO_OVERLAY_HPP
