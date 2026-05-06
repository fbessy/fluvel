// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "analysis_window.hpp"

#include "analysis_widget.hpp"
#include "color_adapters.hpp"
#include "elapsed_timer.hpp"
#include "hausdorff_distance.hpp"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>

namespace fluvel_app
{

AnalysisWindow::AnalysisWindow(QWidget* parent)
    : QDialog(parent)

{
    intersection_.reserve(10000);

    QSettings settings;

    setWindowTitle(tr("Analysis: Input"));

    if (settings.contains("ui_geometry/analysis_window"))
    {
        restoreGeometry(settings.value("ui_geometry/analysis_window").toByteArray());
    }
    else
    {
        resize(900, 600);
    }

    ///////////////////////////////////////////////////////////////
    ///          Input evaluation QDialog window (this)         ///
    ///////////////////////////////////////////////////////////////

    widget1_ = new AnalysisWidget(this);
    widget2_ = new AnalysisWidget(this);

    QHBoxLayout* lists_select_layout = new QHBoxLayout;
    lists_select_layout->addWidget(widget1_);
    lists_select_layout->addWidget(widget2_);

    computeButton_ = new QPushButton(tr("Compute Hausdorff Distance"));
    computeButton_->setEnabled(false);

    QVBoxLayout* input_layout = new QVBoxLayout;
    input_layout->addLayout(lists_select_layout);
    input_layout->addWidget(computeButton_);

    connect(computeButton_, &QPushButton::clicked, this, &AnalysisWindow::computeHd);

    ///////////////////////////////////////
    ///////////////////////////////////////
    ///////////////////////////////////////

    ///////////////////////////////////////
    ///          result_popup           ///
    ///////////////////////////////////////

    hausdorffLabel_ = new QLabel(this);
    hausdorffLabel_->setText(tr("Hausdorff distance: "));
    hausdorffLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hausdorffLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    hausdorffLabel_->setToolTip(tr("Maximum of the minimum distances between the two shapes."));

    normalizedHausdorffLabel_ = new QLabel(this);
    normalizedHausdorffLabel_->setText(tr("Normalized Hausdorff distance: "));
    normalizedHausdorffLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    normalizedHausdorffLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    normalizedHausdorffLabel_->setToolTip(
        tr("Hausdorff distance normalized by the largest image diagonal."));

    QVBoxLayout* hausdorffLayout = new QVBoxLayout;
    hausdorffLayout->addWidget(hausdorffLabel_);
    hausdorffLayout->addWidget(normalizedHausdorffLabel_);
    QGroupBox* hausdorffGroup = new QGroupBox(tr("Hausdorff measure"));
    hausdorffGroup->setLayout(hausdorffLayout);

    percentileSp_ = new QSpinBox;
    percentileSp_->setSingleStep(1);
    percentileSp_->setMinimum(0);
    percentileSp_->setMaximum(100);
    percentileSp_->setSuffix(" %");
    percentileSp_->setValue(90);
    QFormLayout* percentileLayout = new QFormLayout;
    percentileLayout->addRow(tr("Percentile: "), percentileSp_);

    connect(percentileSp_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &AnalysisWindow::refreshQuantile);

    quantileLabel_ = new QLabel(this);
    quantileLabel_->setText(tr("Hausdorff quantile: "));
    quantileLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    quantileLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    quantileLabel_->setToolTip(
        tr("Uses a percentile of the minimum distances instead of the maximum minimum distance."));

    normalizedQuantileLabel_ = new QLabel(this);
    normalizedQuantileLabel_->setText(tr("Normalized Hausdorff quantile: "));
    normalizedQuantileLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    normalizedQuantileLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    normalizedQuantileLabel_->setToolTip(
        tr("Quantile-based Hausdorff distance normalized by the largest image diagonal."));

    QVBoxLayout* quantileLayout = new QVBoxLayout;
    quantileLayout->addLayout(percentileLayout);
    quantileLayout->addWidget(quantileLabel_);
    quantileLayout->addWidget(normalizedQuantileLabel_);
    QGroupBox* quantileGroup = new QGroupBox(tr("Hausdorff quantile measure"));
    quantileGroup->setLayout(quantileLayout);

    centroidsDistLabel_ = new QLabel(this);
    centroidsDistLabel_->setText(tr("Distance between centroids: "));
    centroidsDistLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    centroidsDistLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    normalizedCentroidsDistLabel_ = new QLabel(this);
    normalizedCentroidsDistLabel_->setText(tr("Normalized distance between centroids: "));
    normalizedCentroidsDistLabel_->setToolTip(
        tr("Distance between centroids normalized by the largest image diagonal."));
    normalizedCentroidsDistLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    normalizedCentroidsDistLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QVBoxLayout* centroidsLayout = new QVBoxLayout;
    centroidsLayout->addWidget(centroidsDistLabel_);
    centroidsLayout->addWidget(normalizedCentroidsDistLabel_);
    QGroupBox* centroidsGroup = new QGroupBox(tr("Shape distances"));
    centroidsGroup->setLayout(centroidsLayout);

    timeLabel_ = new QLabel(this);
    timeLabel_->setText(tr("Computation time: -- s"));
    timeLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    timeLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QVBoxLayout* timeLayout = new QVBoxLayout;
    timeLayout->addWidget(timeLabel_);
    QGroupBox* timeGroup = new QGroupBox(tr("Time"));
    timeGroup->setLayout(timeLayout);

    QVBoxLayout* resultsLayout = new QVBoxLayout;
    resultsLayout->addWidget(hausdorffGroup);
    resultsLayout->addWidget(quantileGroup);
    resultsLayout->addWidget(centroidsGroup);
    resultsLayout->addWidget(timeGroup);
    resultsLayout->addStretch(1);

    ///////////////////////////////////////
    ///////////////////////////////////////
    ///////////////////////////////////////

    checkLists();

    setLayout(input_layout);

    resultsPopup_ = new QDialog(this);
    resultsPopup_->setWindowTitle(tr("Analysis: Results"));
    resultsPopup_->setLayout(resultsLayout);
}

void AnalysisWindow::computeHd()
{
    fluvel_ip::ElapsedTimer timer;
    timer.start();

    hd_ = std::make_unique<fluvel_ip::HausdorffDistance>(widget1_->shape(), widget2_->shape(),
                                                         &intersection_);

    const double elapsed = timer.elapsedSec();

    const float hausdorffDist = hd_->distance();
    const float hausdorffQuantile = hd_->hausdorffQuantile(percentileSp_->value());
    const float centroidsGap = hd_->centroidsDistance();

    factor_ = 100.f / fluvel_ip::Shape::gridDiagonal(
                          std::max(widget1_->imageWidth(), widget2_->imageWidth()),
                          std::max(widget1_->imageHeight(), widget2_->imageHeight()));

    const float hdRatio = factor_ * hausdorffDist;
    const float quantileRatio = factor_ * hausdorffQuantile;
    const float gapRatio = factor_ * centroidsGap;

    // =========================
    // Hausdorff
    // =========================

    hausdorffLabel_->setText(
        tr("Hausdorff distance: %1 px").arg(QString::number(hausdorffDist, 'f', 2)));

    normalizedHausdorffLabel_->setText(
        tr("Normalized Hausdorff distance: %1 %").arg(QString::number(hdRatio, 'f', 2)));

    // =========================
    // Quantile
    // =========================

    quantileLabel_->setText(
        tr("Hausdorff quantile: %1 px").arg(QString::number(hausdorffQuantile, 'f', 2)));

    normalizedQuantileLabel_->setText(
        tr("Normalized Hausdorff quantile: %1 %").arg(QString::number(quantileRatio, 'f', 2)));

    // =========================
    // Centroids
    // =========================

    centroidsDistLabel_->setText(
        tr("Distance between centroids: %1 px").arg(QString::number(centroidsGap, 'f', 2)));

    normalizedCentroidsDistLabel_->setText(
        tr("Normalized distance between centroids: %1 %").arg(QString::number(gapRatio, 'f', 2)));

    // =========================
    // Time
    // =========================

    timeLabel_->setText(tr("Computation time: %1 s").arg(QString::number(elapsed, 'f', 2)));

    resultsPopup_->show();
}

void AnalysisWindow::refreshQuantile(int hundredth)
{
    if (hd_ != nullptr)
    {
        const float hausdorffQuantile = hd_->hausdorffQuantile(hundredth);
        const float quantileRatio = factor_ * hausdorffQuantile;

        quantileLabel_->setText(
            tr("Hausdorff quantile: %1 px").arg(QString::number(hausdorffQuantile, 'f', 2)));

        normalizedQuantileLabel_->setText(
            tr("Normalized Hausdorff quantile: %1 %").arg(QString::number(quantileRatio, 'f', 2)));
    }
}

void AnalysisWindow::checkLists()
{
    if (widget1_->shape().isValid() && widget2_->shape().isValid())
    {
        calculateShapesIntersection();
        computeButton_->setEnabled(true);
    }
    else
    {
        computeButton_->setEnabled(false);
    }
}

void AnalysisWindow::calculateShapesIntersection()
{
    std::size_t size1 = widget1_->shape().points().size();
    std::size_t size2 = widget2_->shape().points().size();

    const fluvel_ip::Shape& smaller_shape = (size1 < size2) ? widget1_->shape() : widget2_->shape();

    const QImage& larger_shape_img = (size1 < size2) ? widget2_->image() : widget1_->image();

    const fluvel_ip::Rgb_uc& chosen_rgb = (size1 < size2) ? widget2_->rgb() : widget1_->rgb();

    QRgb rgb_pix;

    intersection_.clear();

    for (const auto& p : smaller_shape.points())
    {
        if (p.x >= 0 && p.x < larger_shape_img.width() && p.y >= 0 &&
            p.y < larger_shape_img.height())
        {
            rgb_pix = larger_shape_img.pixel(p.x, p.y);

            if (toRgb_uc(rgb_pix) == chosen_rgb)
            {
                intersection_.insert(p);
            }
        }
    }
}

void AnalysisWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;

    settings.setValue("ui_geometry/analysis_window", saveGeometry());

    widget1_->saveSettings();
    widget2_->saveSettings();

    QDialog::closeEvent(event);
}

} // namespace fluvel_app
