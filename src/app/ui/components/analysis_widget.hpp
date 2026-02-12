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

#ifndef ANALYSIS_WIDGET_HPP
#define ANALYSIS_WIDGET_HPP

#include <QWidget>
#include "shape.hpp"
#include "contour_rendering_qimage.hpp"
#include "image_view_listener.hpp"
#include "color.hpp"

QT_BEGIN_NAMESPACE
class QSpinBox;
class QComboBox;
class QMimeData;
class QLabel;
class QPushButton;
QT_END_NAMESPACE

namespace ofeli_app
{

class ImageView;

class AnalysisWidget : public QWidget,
                       public ImageViewListener
{
    Q_OBJECT

public :

    AnalysisWidget(QWidget* parent = nullptr);

    int get_img_width() const { return img_width; }
    int get_img_height() const { return img_height; }

    ofeli_ip::Shape& get_shape() { return shape; }
    const QImage get_image() const { return img; }
    const ofeli_ip::Rgb_uc& get_rgb() const { return rgb; }

    void save_settings() const;
    
private :

    void create_list();

    QLabel* text_list_length;
    QString absolute_name;
    QLabel* name_label;
    ImageView* imageView;
    QPushButton* open_button;

    QComboBox* color_list;
    ofeli_ip::Rgb_uc selected;
    QSpinBox* noise_sp;

    QImage img;
    QImage img_noise;
    int img_width;
    int img_height;

    ofeli_ip::Shape shape;
    ofeli_ip::Rgb_uc rgb;

    QString last_directory_used;
    QStringList name_filters;

    static int count_this;
    int id_this;

private slots :

    void open_filename();
    void open_img();
    void get_list_color();
    void refresh_rgb(int);
    void refresh_img_noise(int noise_percent);
    void onColorPicked(const QColor& color,
                       const QPoint& /*imagePos*/) override;

signals :

    void change_list();

};

}

#endif // ANALYSIS_WIDGET_H
