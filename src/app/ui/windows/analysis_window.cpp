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

    setWindowTitle(tr("Analysis: input"));

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

    computeButton_ = new QPushButton(tr("Compute the Hausdorff distance"));
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
    hausdorffLabel_->setText(tr("Hausdorff distance = "));
    hausdorffLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hausdorffLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    hausdorffRatioLabel_ = new QLabel(this);
    hausdorffRatioLabel_->setText(tr("Hausdorff quantile = "));
    hausdorffRatioLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hausdorffRatioLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QVBoxLayout* hausdorff_layout = new QVBoxLayout;
    hausdorff_layout->addWidget(hausdorffLabel_);
    hausdorff_layout->addWidget(hausdorffRatioLabel_);
    QGroupBox* hausdorff_group = new QGroupBox(tr("Hausdorff measure"));
    hausdorff_group->setLayout(hausdorff_layout);

    hundredthSp_ = new QSpinBox;
    hundredthSp_->setSingleStep(1);
    hundredthSp_->setMinimum(0);
    hundredthSp_->setMaximum(100);
    hundredthSp_->setSuffix(" %");
    hundredthSp_->setValue(90);
    QFormLayout* hundredth_layout = new QFormLayout;
    hundredth_layout->addRow("hundredth =", hundredthSp_);

    connect(hundredthSp_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &AnalysisWindow::refreshQuantile);

    quantileLabel_ = new QLabel(this);
    quantileLabel_->setText(tr("Hausdorff ratio = "));
    quantileLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    quantileLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    quantileRatioLabel_ = new QLabel(this);
    quantileRatioLabel_->setText(tr("Hausdorff quantile ratio = "));
    quantileRatioLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    quantileRatioLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QVBoxLayout* quantile_layout = new QVBoxLayout;
    quantile_layout->addLayout(hundredth_layout);
    quantile_layout->addWidget(quantileLabel_);
    quantile_layout->addWidget(quantileRatioLabel_);
    QGroupBox* quantile_group = new QGroupBox(tr("Hausdorff quantile measure"));
    quantile_group->setLayout(quantile_layout);

    centroidsDistLabel_ = new QLabel(this);
    centroidsDistLabel_->setText(tr("distance between centroids = "));
    centroidsDistLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    centroidsDistLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    centroidsRatioLabel_ = new QLabel(this);
    centroidsRatioLabel_->setText(tr("ratio between centroids = "));
    centroidsRatioLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    centroidsRatioLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QVBoxLayout* centroids_layout = new QVBoxLayout;
    centroids_layout->addWidget(centroidsDistLabel_);
    centroids_layout->addWidget(centroidsRatioLabel_);
    QGroupBox* centroids_group = new QGroupBox(tr("Shapes gap"));
    centroids_group->setLayout(centroids_layout);

    timeLabel_ = new QLabel(this);
    timeLabel_->setText(tr("Computation time: -- s"));
    timeLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    timeLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QVBoxLayout* time_layout = new QVBoxLayout;
    time_layout->addWidget(timeLabel_);
    QGroupBox* time_group = new QGroupBox(tr("Time"));
    time_group->setLayout(time_layout);

    QVBoxLayout* result_layout = new QVBoxLayout;
    result_layout->addWidget(hausdorff_group);
    result_layout->addWidget(quantile_group);
    result_layout->addWidget(centroids_group);
    result_layout->addWidget(time_group);
    result_layout->addStretch(1);

    ///////////////////////////////////////
    ///////////////////////////////////////
    ///////////////////////////////////////

    checkLists();

    setLayout(input_layout);

    resultPopup_ = new QDialog(this);
    resultPopup_->setWindowTitle(tr("Analysis: result"));
    resultPopup_->setLayout(result_layout);
}

void AnalysisWindow::computeHd()
{
    fluvel_ip::ElapsedTimer timer;
    timer.start();

    hd_ = std::make_unique<fluvel_ip::HausdorffDistance>(widget1_->shape(), widget2_->shape(),
                                                         &intersection_);

    const double elapsed = timer.elapsedSec();

    const float hausdorffDist = hd_->distance();
    const float hausdorffQuantile = hd_->hausdorffQuantile(hundredthSp_->value());
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

    hausdorffRatioLabel_->setText(
        tr("Hausdorff ratio: %1 %").arg(QString::number(hdRatio, 'f', 2)));

    // =========================
    // Quantile
    // =========================

    quantileLabel_->setText(
        tr("Hausdorff quantile: %1 px").arg(QString::number(hausdorffQuantile, 'f', 2)));

    quantileRatioLabel_->setText(
        tr("Hausdorff quantile ratio: %1 %").arg(QString::number(quantileRatio, 'f', 2)));

    // =========================
    // Centroids
    // =========================

    centroidsDistLabel_->setText(
        tr("Centroids distance: %1 px").arg(QString::number(centroidsGap, 'f', 2)));

    centroidsRatioLabel_->setText(
        tr("Centroids ratio: %1 %").arg(QString::number(gapRatio, 'f', 2)));

    // =========================
    // Time
    // =========================

    timeLabel_->setText(tr("Computation time: %1 s").arg(QString::number(elapsed, 'f', 2)));

    resultPopup_->show();
}

void AnalysisWindow::refreshQuantile(int hundredth)
{
    if (hd_ != nullptr)
    {
        const float hausdorffQuantile = hd_->hausdorffQuantile(hundredth);
        const float quantileRatio = factor_ * hausdorffQuantile;

        quantileLabel_->setText(
            tr("Hausdorff quantile: %1 px").arg(QString::number(hausdorffQuantile, 'f', 2)));

        quantileRatioLabel_->setText(
            tr("Hausdorff quantile ratio: %1 %").arg(QString::number(quantileRatio, 'f', 2)));
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
