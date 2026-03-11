// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "analysis_window.hpp"
#include "analysis_widget.hpp"
#include "color_adapters.hpp"
#include "hausdorff_distance.hpp"

#include <QtWidgets>
#include <ctime> // for std::clock_t, std::clock() and CLOCKS_PER_SEC

namespace fluvel_app
{

AnalysisWindow::AnalysisWindow(QWidget* parent)
    : QDialog(parent)

{
    intersection_.reserve(10000);

    QSettings settings;

    setWindowTitle(tr("Analysis : input"));

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

    computeButton_ = new QPushButton(tr("compute the Hausdorff distance"));
    computeButton_->setEnabled(false);

    QVBoxLayout* input_layout = new QVBoxLayout;
    input_layout->addLayout(lists_select_layout);
    input_layout->addWidget(computeButton_);

    connect(computeButton_, &QPushButton::clicked, this, &AnalysisWindow::compute_hd);

    ///////////////////////////////////////
    ///////////////////////////////////////
    ///////////////////////////////////////

    ///////////////////////////////////////
    ///          result_popup           ///
    ///////////////////////////////////////

    hausdorffLabel_ = new QLabel(this);
    hausdorffLabel_->setText(tr("Hausdorff distance = "));
    hausdorffLabel_->setAlignment(Qt::AlignCenter);
    hausdorffRatioLabel_ = new QLabel(this);
    hausdorffRatioLabel_->setText(tr("Hausdorff quantile = "));
    hausdorffRatioLabel_->setAlignment(Qt::AlignCenter);
    QVBoxLayout* hausdorff_layout = new QVBoxLayout;
    hausdorff_layout->addWidget(hausdorffLabel_);
    hausdorff_layout->addWidget(hausdorffRatioLabel_);
    QGroupBox* hausdorff_group = new QGroupBox(tr("Hausdorff measure"));
    hausdorff_group->setLayout(hausdorff_layout);

    hundredthSp_ = new QSpinBox;
    hundredthSp_->setSingleStep(1);
    hundredthSp_->setMinimum(0);
    hundredthSp_->setMaximum(100);
    hundredthSp_->setSuffix(tr(" %"));
    hundredthSp_->setValue(90);
    QFormLayout* hundredth_layout = new QFormLayout;
    hundredth_layout->addRow("hundredth =", hundredthSp_);

    connect(hundredthSp_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &AnalysisWindow::refresh_quantile);

    quantileLabel_ = new QLabel(this);
    quantileLabel_->setText(tr("Hausdorff ratio = "));
    quantileLabel_->setAlignment(Qt::AlignCenter);
    quantileRatioLabel_ = new QLabel(this);
    quantileRatioLabel_->setText(tr("Hausdorff quantile ratio = "));
    quantileRatioLabel_->setAlignment(Qt::AlignCenter);
    QVBoxLayout* quantile_layout = new QVBoxLayout;
    quantile_layout->addLayout(hundredth_layout);
    quantile_layout->addWidget(quantileLabel_);
    quantile_layout->addWidget(quantileRatioLabel_);
    QGroupBox* quantile_group = new QGroupBox(tr("Hausdorff quantile measure"));
    quantile_group->setLayout(quantile_layout);

    centroidsDistLabel_ = new QLabel(this);
    centroidsDistLabel_->setText(tr("distance between centroids = "));
    centroidsDistLabel_->setAlignment(Qt::AlignCenter);
    centroids_ratio_label_ = new QLabel(this);
    centroids_ratio_label_->setText(tr("ratio between centroids = "));
    centroids_ratio_label_->setAlignment(Qt::AlignCenter);
    QVBoxLayout* centroids_layout = new QVBoxLayout;
    centroids_layout->addWidget(centroidsDistLabel_);
    centroids_layout->addWidget(centroids_ratio_label_);
    QGroupBox* centroids_group = new QGroupBox(tr("Shapes gap"));
    centroids_group->setLayout(centroids_layout);

    timeLabel_ = new QLabel(this);
    timeLabel_->setText(tr("Calculating time = "));
    timeLabel_->setAlignment(Qt::AlignCenter);
    QVBoxLayout* time_layout = new QVBoxLayout;
    time_layout->addWidget(timeLabel_);
    QGroupBox* time_group = new QGroupBox(tr("Calculating time"));
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

    check_lists();

    setLayout(input_layout);

    resultPopup_ = new QDialog(this);
    resultPopup_->setWindowTitle(tr("Analysis : result"));
    resultPopup_->setLayout(result_layout);
}

void AnalysisWindow::compute_hd()
{
    if (hd_ != nullptr)
    {
        delete hd_;
        hd_ = nullptr;
    }

    float elapsed = std::numeric_limits<float>().max();

    if (hd_ == nullptr)
    {
        std::clock_t start_time = std::clock();

        hd_ = new fluvel_ip::HausdorffDistance(widget1_->get_shape(), widget2_->get_shape(),
                                              intersection_);

        elapsed = float(std::clock() - start_time) / float(CLOCKS_PER_SEC);
    }

    float hausdorff_dist = hd_->get_distance();
    float hausdorffQuantile = hd_->hausdorffQuantile(hundredthSp_->value());
    float centr_gap = hd_->get_centroids_distance();

    factor_ = 100.f / fluvel_ip::Shape::get_grid_diagonal(
                          std::max(widget1_->get_img_width(), widget2_->get_img_width()),
                          std::max(widget1_->get_img_height(), widget2_->get_img_height()));

    float hd_ratio = factor_ * hausdorff_dist;
    float quantile_ratio = factor_ * hausdorffQuantile;
    float gap_ratio = factor_ * centr_gap;

    hausdorffLabel_->setText(tr("Hausdorff distance = ") + QString::number(hausdorff_dist) +
                              (tr(" pixels")));
    hausdorffRatioLabel_->setText(tr("Hausdorff ratio = ") + QString::number(hd_ratio) + (" %"));

    quantileLabel_->setText(tr("Hausdorff quantile = ") + QString::number(hausdorffQuantile) +
                             (tr(" pixels")));
    quantileRatioLabel_->setText(tr("Hausdorff quantile ratio = ") +
                                   QString::number(quantile_ratio) + (" %"));

    centroidsDistLabel_->setText(tr("distance between centroids = ") +
                                   QString::number(centr_gap) + (tr(" pixels")));
    centroids_ratio_label_->setText(tr("ratio between centroids = ") + QString::number(gap_ratio) +
                                    (" %"));

    timeLabel_->setText(tr("time = ") + QString::number(elapsed, 'g', 4) + (" s"));

    resultPopup_->show();
}

void AnalysisWindow::refresh_quantile(int hundredth)
{
    if (hd_ != nullptr)
    {
        float hausdorffQuantile = hd_->hausdorffQuantile(hundredth);
        float quantile_ratio = factor_ * hausdorffQuantile;

        quantileLabel_->setText(tr("Hausdorff quantile = ") + QString::number(hausdorffQuantile) +
                                 (tr(" pixels")));
        quantileRatioLabel_->setText(tr("Hausdorff quantile ratio = ") +
                                       QString::number(quantile_ratio) + (" %"));
    }
}

void AnalysisWindow::check_lists()
{
    if (widget1_->get_shape().is_valid() && widget2_->get_shape().is_valid())
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
    std::size_t size1 = widget1_->get_shape().get_points().size();
    std::size_t size2 = widget2_->get_shape().get_points().size();

    const fluvel_ip::Shape& smaller_shape =
        (size1 < size2) ? widget1_->get_shape() : widget2_->get_shape();

    const QImage& larger_shape_img =
        (size1 < size2) ? widget2_->get_image() : widget1_->get_image();

    const fluvel_ip::Rgb_uc& chosen_rgb =
        (size1 < size2) ? widget2_->get_rgb() : widget1_->get_rgb();

    QRgb rgb_pix;

    intersection_.clear();

    for (const auto& p : smaller_shape.get_points())
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

    widget1_->save_settings();
    widget2_->save_settings();

    QDialog::closeEvent(event);
}

} // namespace fluvel_app
