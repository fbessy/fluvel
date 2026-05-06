// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "analysis_widget.hpp"
#include "hausdorff_distance.hpp"
#include "point_containers.hpp"

#include <QDialog>

class QWidget;
class QCloseEvent;
class QPushButton;
class QLabel;
class QSpinBox;

namespace fluvel_app
{

class AnalysisWidget;

/**
 * @brief Dialog for comparing analysis results between two images.
 *
 * This window allows the user to compare shapes extracted from two
 * AnalysisWidget instances. It computes various metrics such as:
 * - Hausdorff distance
 * - quantile distance
 * - centroid distance
 * - shape intersection
 *
 * Results are displayed in a dedicated popup.
 */
class AnalysisWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the analysis window.
     *      * @param parent Optional parent widget.
     */
    AnalysisWindow(QWidget* parent = nullptr);
    void checkLists();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void computeHd();
    void refreshQuantile(int hundredth);

    void calculateShapesIntersection();

    AnalysisWidget* widget1_ = nullptr;
    AnalysisWidget* widget2_ = nullptr;
    QPushButton* computeButton_ = nullptr;

    fluvel_ip::PointSet intersection_;

    QDialog* resultsPopup_ = nullptr;
    QLabel* hausdorffLabel_ = nullptr;
    QSpinBox* percentileSp_ = nullptr;
    QLabel* quantileLabel_ = nullptr;
    QLabel* normalizedHausdorffLabel_ = nullptr;
    QLabel* normalizedQuantileLabel_ = nullptr;
    QLabel* centroidsDistLabel_ = nullptr;
    QLabel* normalizedCentroidsDistLabel_ = nullptr;
    QLabel* timeLabel_ = nullptr;

    std::unique_ptr<fluvel_ip::HausdorffDistance> hd_;
    float factor_{0.f};
};

} // namespace fluvel_app
