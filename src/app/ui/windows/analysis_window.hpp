// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "analysis_widget.hpp"
#include "hausdorff_distance.hpp"
#include "point.hpp"

#include <QDialog>
#include <QLabel>
#include <QSpinBox>

namespace fluvel_app
{

class AnalysisWindow : public QDialog
{
    Q_OBJECT

public:
    //! A parametric constructor with a pointer on the QWidget parent.
    AnalysisWindow(QWidget* parent);

public slots:

    void check_lists();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void calculateShapesIntersection();

    AnalysisWidget* widget1_;
    AnalysisWidget* widget2_;
    QPushButton* compute_button_;

    std::unordered_set<fluvel_ip::Point2D_i> intersection_;

    QDialog* result_popup_;
    QLabel* hausdorff_label_;
    QSpinBox* hundredth_sp_;
    QLabel* quantile_label_;
    QLabel* hausdorff_ratio_label_;
    QLabel* quantile_ratio_label_;
    QLabel* centroids_dist_label_;
    QLabel* centroids_ratio_label_;
    QLabel* time_label_;

    fluvel_ip::HausdorffDistance* hd_{nullptr};
    float factor_{0.f};

private slots:

    void compute_hd();
    void refresh_quantile(int hundredth);
};

} // namespace fluvel_app
