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

#ifndef ANALYSIS_WINDOW_HPP
#define ANALYSIS_WINDOW_HPP

#include <QDialog>
#include <QLabel>
#include <QSpinBox>

#include "analysis_widget.hpp"
#include "hausdorff_distance.hpp"
#include "point.hpp"

namespace ofeli_app
{

class AnalysisWindow : public QDialog
{
    Q_OBJECT

public :

    //! A parametric constructor with a pointer on the QWidget parent.
    AnalysisWindow(QWidget* parent);

public slots :

    void check_lists();

protected:
    void closeEvent(QCloseEvent* event) override;

private :

    void calculate_shapes_intersection();

    AnalysisWidget* widget1_;
    AnalysisWidget* widget2_;
    QPushButton* compute_button_;

    std::unordered_set<ofeli_ip::Point2D_i> intersection_;

    QDialog* result_popup_;
    QLabel* hausdorff_label_;
    QSpinBox* hundredth_sp_;
    QLabel* quantile_label_;
    QLabel* hausdorff_ratio_label_;
    QLabel* quantile_ratio_label_;
    QLabel* centroids_dist_label_;
    QLabel* centroids_ratio_label_;
    QLabel* time_label_;

    ofeli_ip::HausdorffDistance* hd_{nullptr};
    float factor_{0.f};

private slots :

    void compute_hd();
    void refresh_quantile(int hundredth);
};

}

#endif // ANALYSIS_WINDOW_HPP

//! \class ofeli::AnalysisWindow
//! The class AnalysisWindow is a QDialog window that informs the user about Ofeli . An instance of this class is created by #ofeli::ImageWindow and displayed when the user clicks on menu About.
