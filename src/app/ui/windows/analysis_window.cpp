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

    compute_button_ = new QPushButton(tr("compute the Hausdorff distance"));
    compute_button_->setEnabled(false);

    QVBoxLayout* input_layout = new QVBoxLayout;
    input_layout->addLayout(lists_select_layout);
    input_layout->addWidget(compute_button_);

    connect(compute_button_, &QPushButton::clicked, this, &AnalysisWindow::compute_hd);

    ///////////////////////////////////////
    ///////////////////////////////////////
    ///////////////////////////////////////

    ///////////////////////////////////////
    ///          result_popup           ///
    ///////////////////////////////////////

    hausdorff_label_ = new QLabel(this);
    hausdorff_label_->setText(tr("Hausdorff distance = "));
    hausdorff_label_->setAlignment(Qt::AlignCenter);
    hausdorff_ratio_label_ = new QLabel(this);
    hausdorff_ratio_label_->setText(tr("Hausdorff quantile = "));
    hausdorff_ratio_label_->setAlignment(Qt::AlignCenter);
    QVBoxLayout* hausdorff_layout = new QVBoxLayout;
    hausdorff_layout->addWidget(hausdorff_label_);
    hausdorff_layout->addWidget(hausdorff_ratio_label_);
    QGroupBox* hausdorff_group = new QGroupBox(tr("Hausdorff measure"));
    hausdorff_group->setLayout(hausdorff_layout);

    hundredth_sp_ = new QSpinBox;
    hundredth_sp_->setSingleStep(1);
    hundredth_sp_->setMinimum(0);
    hundredth_sp_->setMaximum(100);
    hundredth_sp_->setSuffix(tr(" %"));
    hundredth_sp_->setValue(90);
    QFormLayout* hundredth_layout = new QFormLayout;
    hundredth_layout->addRow("hundredth =", hundredth_sp_);

    connect(hundredth_sp_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &AnalysisWindow::refresh_quantile);

    quantile_label_ = new QLabel(this);
    quantile_label_->setText(tr("Hausdorff ratio = "));
    quantile_label_->setAlignment(Qt::AlignCenter);
    quantile_ratio_label_ = new QLabel(this);
    quantile_ratio_label_->setText(tr("Hausdorff quantile ratio = "));
    quantile_ratio_label_->setAlignment(Qt::AlignCenter);
    QVBoxLayout* quantile_layout = new QVBoxLayout;
    quantile_layout->addLayout(hundredth_layout);
    quantile_layout->addWidget(quantile_label_);
    quantile_layout->addWidget(quantile_ratio_label_);
    QGroupBox* quantile_group = new QGroupBox(tr("Hausdorff quantile measure"));
    quantile_group->setLayout(quantile_layout);

    centroids_dist_label_ = new QLabel(this);
    centroids_dist_label_->setText(tr("distance between centroids = "));
    centroids_dist_label_->setAlignment(Qt::AlignCenter);
    centroids_ratio_label_ = new QLabel(this);
    centroids_ratio_label_->setText(tr("ratio between centroids = "));
    centroids_ratio_label_->setAlignment(Qt::AlignCenter);
    QVBoxLayout* centroids_layout = new QVBoxLayout;
    centroids_layout->addWidget(centroids_dist_label_);
    centroids_layout->addWidget(centroids_ratio_label_);
    QGroupBox* centroids_group = new QGroupBox(tr("Shapes gap"));
    centroids_group->setLayout(centroids_layout);

    time_label_ = new QLabel(this);
    time_label_->setText(tr("Calculating time = "));
    time_label_->setAlignment(Qt::AlignCenter);
    QVBoxLayout* time_layout = new QVBoxLayout;
    time_layout->addWidget(time_label_);
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

    result_popup_ = new QDialog(this);
    result_popup_->setWindowTitle(tr("Analysis : result"));
    result_popup_->setLayout(result_layout);
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
    float hausdorffQuantile = hd_->hausdorffQuantile(hundredth_sp_->value());
    float centr_gap = hd_->get_centroids_distance();

    factor_ = 100.f / fluvel_ip::Shape::get_grid_diagonal(
                          std::max(widget1_->get_img_width(), widget2_->get_img_width()),
                          std::max(widget1_->get_img_height(), widget2_->get_img_height()));

    float hd_ratio = factor_ * hausdorff_dist;
    float quantile_ratio = factor_ * hausdorffQuantile;
    float gap_ratio = factor_ * centr_gap;

    hausdorff_label_->setText(tr("Hausdorff distance = ") + QString::number(hausdorff_dist) +
                              (tr(" pixels")));
    hausdorff_ratio_label_->setText(tr("Hausdorff ratio = ") + QString::number(hd_ratio) + (" %"));

    quantile_label_->setText(tr("Hausdorff quantile = ") + QString::number(hausdorffQuantile) +
                             (tr(" pixels")));
    quantile_ratio_label_->setText(tr("Hausdorff quantile ratio = ") +
                                   QString::number(quantile_ratio) + (" %"));

    centroids_dist_label_->setText(tr("distance between centroids = ") +
                                   QString::number(centr_gap) + (tr(" pixels")));
    centroids_ratio_label_->setText(tr("ratio between centroids = ") + QString::number(gap_ratio) +
                                    (" %"));

    time_label_->setText(tr("time = ") + QString::number(elapsed, 'g', 4) + (" s"));

    result_popup_->show();
}

void AnalysisWindow::refresh_quantile(int hundredth)
{
    if (hd_ != nullptr)
    {
        float hausdorffQuantile = hd_->hausdorffQuantile(hundredth);
        float quantile_ratio = factor_ * hausdorffQuantile;

        quantile_label_->setText(tr("Hausdorff quantile = ") + QString::number(hausdorffQuantile) +
                                 (tr(" pixels")));
        quantile_ratio_label_->setText(tr("Hausdorff quantile ratio = ") +
                                       QString::number(quantile_ratio) + (" %"));
    }
}

void AnalysisWindow::check_lists()
{
    if (widget1_->get_shape().is_valid() && widget2_->get_shape().is_valid())
    {
        calculateShapesIntersection();
        compute_button_->setEnabled(true);
    }
    else
    {
        compute_button_->setEnabled(false);
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
