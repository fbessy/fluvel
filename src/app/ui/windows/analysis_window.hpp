// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "analysis_widget.hpp"
#include "point_containers.hpp"

#include <QDialog>

class QWidget;
class QCloseEvent;
class QPushButton;
class QLabel;
class QSpinBox;

namespace fluvel_ip
{
class HausdorffDistance;
}

namespace fluvel_app
{

class AnalysisWidget;

class AnalysisWindow : public QDialog
{
    Q_OBJECT

public:
    //! A parametric constructor with a pointer on the QWidget parent.
    AnalysisWindow(QWidget* parent);

    void check_lists();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void compute_hd();
    void refresh_quantile(int hundredth);

    void calculateShapesIntersection();

    AnalysisWidget* widget1_ = nullptr;
    AnalysisWidget* widget2_ = nullptr;
    QPushButton* computeButton_ = nullptr;

    fluvel_ip::PointSet intersection_;

    QDialog* resultPopup_ = nullptr;
    QLabel* hausdorffLabel_ = nullptr;
    QSpinBox* hundredthSp_ = nullptr;
    QLabel* quantileLabel_ = nullptr;
    QLabel* hausdorffRatioLabel_ = nullptr;
    QLabel* quantileRatioLabel_ = nullptr;
    QLabel* centroidsDistLabel_ = nullptr;
    QLabel* centroidsRatio_Label_ = nullptr;
    QLabel* timeLabel_ = nullptr;

    fluvel_ip::HausdorffDistance* hd_ = nullptr;
    float factor_{0.f};
};

} // namespace fluvel_app
