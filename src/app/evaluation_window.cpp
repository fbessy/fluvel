/****************************************************************************
**
** Copyright (C) 2010-2025 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.
**
** http://www.cecill.info/licences/Licence_CeCILL_V2-en.html
** You may use this file under the terms of the CeCILL license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Fabien Bessy and its Subsidiary(-ies) nor the
**     names of its contributors may be used to endorse or promote products
**     derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

#include "evaluation_window.hpp"
#include "evaluation_widget.hpp"
#include "hausdorff_distance.hpp"

#include <QtWidgets>
#include <ctime>       // for std::clock_t, std::clock() and CLOCKS_PER_SEC

namespace ofeli_gui
{

EvaluationWindow::EvaluationWindow(QWidget* parent) :
    QDialog(parent),
    hd(nullptr),
    factor(0.f)
{
    intersection.reserve(10000);

    QSettings settings;

    setWindowTitle(tr("Evaluation : input"));

    const auto geo = settings.value("Evaluation/Window/geometry").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    ///////////////////////////////////////////////////////////////
    ///          Input evaluation QDialog window (this)         ///
    ///////////////////////////////////////////////////////////////

    widget1 = new EvaluationWidget(this);
    widget2 = new EvaluationWidget(this);


    QHBoxLayout* lists_select_layout = new QHBoxLayout;
    lists_select_layout->addWidget(widget1);
    lists_select_layout->addWidget(widget2);


    compute_button = new QPushButton(tr("compute the Hausdorff distance"));
    compute_button->setEnabled(false);

    QVBoxLayout* input_layout = new QVBoxLayout;
    input_layout->addLayout(lists_select_layout);
    input_layout->addWidget(compute_button);

    connect( compute_button, SIGNAL(clicked()), this, SLOT(compute_hd()) );


    ///////////////////////////////////////
    ///////////////////////////////////////
    ///////////////////////////////////////


    ///////////////////////////////////////
    ///          result_popup           ///
    ///////////////////////////////////////

    hausdorff_label = new QLabel(this);
    hausdorff_label->setText( tr("Hausdorff distance = ") );
    hausdorff_label->setAlignment(Qt::AlignCenter);
    hausdorff_ratio_label = new QLabel(this);
    hausdorff_ratio_label->setText( tr("Hausdorff quantile = ") );
    hausdorff_ratio_label->setAlignment(Qt::AlignCenter);
    QVBoxLayout* hausdorff_layout = new QVBoxLayout;
    hausdorff_layout->addWidget(hausdorff_label);
    hausdorff_layout->addWidget(hausdorff_ratio_label);
    QGroupBox* hausdorff_group = new QGroupBox( tr("Hausdorff measure") );
    hausdorff_group->setLayout(hausdorff_layout);

    hundredth_sp = new QSpinBox;
    hundredth_sp->setSingleStep(1);
    hundredth_sp->setMinimum(0);
    hundredth_sp->setMaximum(100);
    hundredth_sp->setSuffix(tr(" %"));
    hundredth_sp->setValue(90);
    QFormLayout* hundredth_layout = new QFormLayout;
    hundredth_layout->addRow("hundredth =", hundredth_sp);

    connect( hundredth_sp, SIGNAL(valueChanged(int)), this, SLOT(refresh_quantile(int)) );

    quantile_label = new QLabel(this);
    quantile_label->setText( tr("Hausdorff ratio = ") );
    quantile_label->setAlignment(Qt::AlignCenter);
    quantile_ratio_label = new QLabel(this);
    quantile_ratio_label->setText( tr("Hausdorff quantile ratio = ") );
    quantile_ratio_label->setAlignment(Qt::AlignCenter);
    QVBoxLayout* quantile_layout = new QVBoxLayout;
    quantile_layout->addLayout(hundredth_layout);
    quantile_layout->addWidget(quantile_label);
    quantile_layout->addWidget(quantile_ratio_label);
    QGroupBox* quantile_group = new QGroupBox( tr("Hausdorff quantile measure") );
    quantile_group->setLayout(quantile_layout);

    centroids_dist_label = new QLabel(this);
    centroids_dist_label->setText( tr("distance between centroids = ") );
    centroids_dist_label->setAlignment(Qt::AlignCenter);
    centroids_ratio_label = new QLabel(this);
    centroids_ratio_label->setText( tr("ratio between centroids = ") );
    centroids_ratio_label->setAlignment(Qt::AlignCenter);
    QVBoxLayout* centroids_layout = new QVBoxLayout;
    centroids_layout->addWidget(centroids_dist_label);
    centroids_layout->addWidget(centroids_ratio_label);
    QGroupBox* centroids_group = new QGroupBox( tr("Shapes gap") );
    centroids_group->setLayout(centroids_layout);

    time_label = new QLabel(this);
    time_label->setText( tr("Calculating time = ") );
    time_label->setAlignment(Qt::AlignCenter);
    QVBoxLayout* time_layout = new QVBoxLayout;
    time_layout->addWidget(time_label);
    QGroupBox* time_group = new QGroupBox( tr("Calculating time") );
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

    result_popup = new QDialog(this);
    result_popup->setWindowTitle(tr("Evaluation : result"));
    result_popup->setLayout(result_layout);
}

void EvaluationWindow::compute_hd()
{
    if( hd != nullptr )
    {
        delete hd;
        hd = nullptr;
    }

    float elapsed = std::numeric_limits<float>().max();

    if( hd == nullptr )
    {
        std::clock_t start_time = std::clock();

        hd = new ofeli_ip::HausdorffDistance( widget1->get_shape(),
                                              widget2->get_shape(),
                                              intersection );

        elapsed = float(std::clock() - start_time) / float(CLOCKS_PER_SEC);
    }

    float hausdorff_dist = hd->get_distance();
    float hausdorff_quantile = hd->get_hausdorff_quantile( hundredth_sp->value() );
    float centr_gap = hd->get_centroids_distance();

    factor = 100.f / ofeli_ip::Shape::get_grid_diagonal(std::max(widget1->get_img_width(), widget2->get_img_width()),
                                                        std::max(widget1->get_img_height(), widget2->get_img_height()));

    float hd_ratio         = factor * hausdorff_dist;
    float quantile_ratio   = factor * hausdorff_quantile;
    float gap_ratio        = factor * centr_gap;

    hausdorff_label->setText(tr("Hausdorff distance = ")
                             +QString::number(hausdorff_dist)+(tr(" pixels")));
    hausdorff_ratio_label->setText(tr("Hausdorff ratio = ")
                                   +QString::number(hd_ratio)+(" %"));

    quantile_label->setText(tr("Hausdorff quantile = ")
                            +QString::number(hausdorff_quantile)+(tr(" pixels")));
    quantile_ratio_label->setText(tr("Hausdorff quantile ratio = ")
                                  +QString::number(quantile_ratio)+(" %"));

    centroids_dist_label->setText(tr("distance between centroids = ")
                                  +QString::number(centr_gap)+(tr(" pixels")));
    centroids_ratio_label->setText(tr("ratio between centroids = ")
                                   +QString::number(gap_ratio)+(" %"));

    time_label->setText(tr("time = ")+QString::number(elapsed, 'g', 4)+(" s"));

    result_popup->show();
}

void EvaluationWindow::refresh_quantile(int hundredth)
{
    if( hd != nullptr )
    {
        float hausdorff_quantile = hd->get_hausdorff_quantile( hundredth );
        float quantile_ratio   = factor * hausdorff_quantile;

        quantile_label->setText(tr("Hausdorff quantile = ")
                                +QString::number(hausdorff_quantile)+(tr(" pixels")));
        quantile_ratio_label->setText(tr("Hausdorff quantile ratio = ")
                                      +QString::number(quantile_ratio)+(" %"));
    }
}

void EvaluationWindow::check_lists()
{
    if( widget1->get_shape().is_valid() &&
        widget2->get_shape().is_valid()    )
    {
        calculate_shapes_intersection();
        compute_button->setEnabled(true);
    }
    else
    {
        compute_button->setEnabled(false);
    }
}

void EvaluationWindow::calculate_shapes_intersection()
{
    std::size_t size1 = widget1->get_shape().get_points().size();
    std::size_t size2 = widget2->get_shape().get_points().size();

    const ofeli_ip::Shape& smaller_shape = ( size1 < size2 ) ?
                                           widget1->get_shape() : widget2->get_shape();

    const QImage& larger_shape_img = ( size1 < size2 ) ?
                                     widget2->get_image() : widget1->get_image();

    const RgbColor& chosen_rgb = ( size1 < size2 ) ?
                                 widget2->get_rgb() : widget1->get_rgb();

    QRgb rgb_pix;

    intersection.clear();

    for( const auto& p : smaller_shape.get_points() )
    {
        if ( p.x >= 0 &&
             p.x < larger_shape_img.width() &&
             p.y >= 0 &&
             p.y < larger_shape_img.height() )
        {
            rgb_pix = larger_shape_img.pixel(p.x, p.y);

            if( (unsigned char)(qRed(rgb_pix))   == chosen_rgb.red   &&
                (unsigned char)(qGreen(rgb_pix)) == chosen_rgb.green &&
                (unsigned char)(qBlue(rgb_pix)   == chosen_rgb.blue)    )
            {
                intersection.insert( p );
            }
        }
    }
}

void EvaluationWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;

    settings.setValue( "Evaluation/Window/geometry", saveGeometry() );

    widget1->save_settings();
    widget2->save_settings();

    QDialog::closeEvent(event);
}

}
