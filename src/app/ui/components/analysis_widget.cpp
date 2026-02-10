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

#include "analysis_widget.hpp"
#include "analysis_window.hpp"
#include "interaction_set.hpp"
#include "color_picker_behavior.hpp"
#include "pan_behavior.hpp"

#include "image_view.hpp"
#include <QtWidgets>

namespace ofeli_app
{

int AnalysisWidget::count_this = 0;

AnalysisWidget::AnalysisWidget(QWidget *parent) :
    QWidget(parent),
    img_width(0), img_height(0)
{
    ++count_this; // static variable to count the instances
    id_this = count_this; // in order to know if *this is the first or the second widget of evaluation_window

    QSettings settings;

    text_list_length = new QLabel(this);
    text_list_length->setAlignment(Qt::AlignCenter);
    if( id_this == 1 )
    {
        text_list_length->setText("<font color=red>"+tr("List 1 length = ")+QString::number(shape.get_points().size()));
    }
    else if( id_this == 2 )
    {
        text_list_length->setText("<font color=red>"+tr("List 2 length = ")+QString::number(shape.get_points().size()));
    }

    /////////////////////////////////////////////////////////////////////////////////

    name_label = new QLabel(this);
    name_label->setText( tr("Title - Size") );
    name_label->setAlignment(Qt::AlignCenter);

    ///////////////////////////////////////

    imageView = new ImageView(this);
    auto interaction = std::make_unique<InteractionSet>();
    //interaction->addBehavior(std::make_unique<AutoFitBehavior>());
    //interaction->addBehavior(std::make_unique<FullscreenBehavior>());
    interaction->addBehavior(std::make_unique<PanBehavior>());
    interaction->addBehavior(std::make_unique<ColorPickerBehavior>());
    imageView->setInteraction(interaction.release());

    imageView->setListener(this);

    ///////////////////////////////////////

    QPushButton* open_button = new QPushButton( tr("Open image") + " " +
                                                QString::number(id_this) );

    QVBoxLayout* img_layout = new QVBoxLayout;
    img_layout->addWidget(name_label);
    img_layout->addWidget(imageView);
    img_layout->addWidget(open_button);
    QGroupBox* img_group = new QGroupBox( tr("Image") + " " +
                                          QString::number(id_this) );
    img_group->setLayout(img_layout);

    //////////////////////////////////////////////////////////////////////////////////

    color_list = new QComboBox;

    QPixmap pm(12,12);

    pm.fill(Qt::red);
    color_list->addItem( pm, tr("Red") );
    pm.fill(Qt::green);
    color_list->addItem( pm, tr("Green") );
    pm.fill(Qt::blue);
    color_list->addItem( pm, tr("Blue") );
    pm.fill(Qt::cyan);
    color_list->addItem( pm, tr("Cyan") );
    pm.fill(Qt::magenta);
    color_list->addItem( pm, tr("Magenta") );
    pm.fill(Qt::yellow);
    color_list->addItem( pm, tr("Yellow") );
    pm.fill(Qt::black);
    color_list->addItem( pm, tr("Black") );
    pm.fill(Qt::white);
    color_list->addItem( pm, tr("White") );

    selected.red = (unsigned char)( settings.value("Analysis/R"
                                                   +QString::number(id_this), 128).toInt() );
    selected.green = (unsigned char)( settings.value("Analysis/G"
                                                     +QString::number(id_this), 0).toInt() );
    selected.blue = (unsigned char)( settings.value("Analysis/B"
                                                    +QString::number(id_this), 255).toInt() );

    pm.fill( QColor(selected.red,
                    selected.green,
                    selected.blue) );
    color_list->addItem( pm, tr("Selected") );

    color_list->setCurrentIndex( settings.value("Analysis/combo"
                                                +QString::number(id_this), 0).toInt() );

    ///////////////////////////////////////

    QPushButton* color_select = new QPushButton( tr("Select") );

    QFormLayout* form = new QFormLayout;
    form->addRow(tr("List from :"), color_list);
    form->addRow(tr("<click on image> |"), color_select);

    QGroupBox* color_group = new QGroupBox( tr("Color") + " "
                                            + QString::number(id_this) );
    color_group->setLayout(form);

    noise_sp = new QSpinBox;
    noise_sp->setSingleStep(1);
    noise_sp->setMinimum(0);
    noise_sp->setMaximum(100);
    noise_sp->setSuffix(tr(" %"));
    noise_sp->setValue(0);
    QFormLayout* noise_layout = new QFormLayout;
    noise_layout->addRow("noise =", noise_sp);

    QVBoxLayout* this_layout = new QVBoxLayout;
    this_layout->addWidget(text_list_length);
    this_layout->addWidget(img_group);
    this_layout->addWidget(color_group);
    this_layout->addLayout(noise_layout);

    setLayout(this_layout);

    last_directory_used = settings.value("Main/Name/last_directory_used",
                                         QDir().homePath()).toString();

    name_filters << "*.bmp"
                    //<< "*.dcm"
                 << "*.gif"
                 << "*.jpg" << "*.jpeg" << "*.mng"
                 << "*.pbm" << "*.png" << "*.pgm"
                 << "*.ppm" << "*.svg" << "*.svgz"
                 << "*.tiff" << "*.tif" << "*.xbm" << "*.xpm";

    name_filters.removeDuplicates();

    imageView->setListener(this);

    connect(open_button, &QPushButton::clicked,
            this, &AnalysisWidget::open_filename);

    connect(color_list,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AnalysisWidget::refresh_rgb);

    connect(color_select, &QPushButton::clicked,
            this, &AnalysisWidget::get_list_color);

    auto *analysisWindow = qobject_cast<AnalysisWindow*>(parentWidget());
    Q_ASSERT(analysisWindow);

    connect(this,   &AnalysisWidget::change_list,
            analysisWindow, &AnalysisWindow::check_lists);

    connect(noise_sp,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AnalysisWidget::refresh_img_noise);
}

void AnalysisWidget::open_filename()
{
    absolute_name = QFileDialog::getOpenFileName(this,
                                                 tr("Open File") + " " + QString::number(id_this),
                                                 last_directory_used,
                                                 tr("Image Files (%1)").arg(name_filters.join(" ")));
    open_img();
}

void AnalysisWidget::open_img()
{
    if( !absolute_name.isEmpty() )
    {
        img = QImage(absolute_name);
        img_height = img.height();
        img_width = img.width();

        if( img.isNull() )
        {
            QMessageBox::information(this, tr("Opening error - Ofeli"),
                                     tr("Cannot load %1.").arg(QDir::toNativeSeparators(absolute_name)));
            return;
        }

        refresh_rgb( color_list->currentIndex() );

        QFileInfo fi(absolute_name);
        QString name = fi.fileName();

        QString string_lists_text;
        string_lists_text=QString::number(img_width)+"×"
                +QString::number(img_height);
        name_label->setText(name +" - "+string_lists_text);
    }
}

void AnalysisWidget::refresh_rgb(int color_list_index)
{
    if ( color_list_index == ComboBoxColorIndex::SELECTED )
    {
        rgb = selected;
    }
    else
    {
        //get_color(color_list_index, rgb);
    }

    refresh_img_noise( noise_sp->value() );
}

void AnalysisWidget::create_list()
{
    shape.clear();

    QRgb pix;

    for( int y = 0; y < img_height; ++y )
    {
        for( int x = 0; x < img_width; ++x )
        {
            pix = img_noise.pixel(x,y);

            if(    (unsigned char)(qRed(pix))   == rgb.red
                && (unsigned char)(qGreen(pix)) == rgb.green
                && (unsigned char)(qBlue(pix)   == rgb.blue) )
            {
                shape.push_back( x, y );
            }
        }
    }

    shape.calculate_centroid();


    QString size_str = QString::number(shape.get_points().size());

    QString color_str;
    if( shape.get_points().empty() )
    {
        color_str = "<font color=red>";
    }
    else
    {
        color_str = "<font color=green>";
    }

    QString list_str;
    if( id_this == 1 )
    {
        list_str = tr("List 1 length = ");
    }
    else if( id_this == 2 )
    {
        list_str = tr("List 2 length = ");
    }

    text_list_length->setText( color_str +
                               list_str  +
                               size_str    );

    emit change_list();
}

void AnalysisWidget::refresh_img_noise(int noise_percent)
{
    if ( !img.isNull() )
    {
        img_noise = img;

        std::random_device rd;
        std::mt19937 gen( rd() );
        std::bernoulli_distribution proba_distri{ float(noise_percent) / 100.f };

        QColor color( int(rgb.red),
                      int(rgb.green),
                      int(rgb.blue) );

        QRgb rgb_color = color.rgb();

        for( int y = 0; y < img_height; ++y )
        {
            for( int x = 0; x < img_width; ++x )
            {
                if ( proba_distri(gen) )
                {
                    img_noise.setPixel(x, y, rgb_color);
                }
            }
        }

        imageView->setImage(img_noise);

        create_list();
    }
}

void AnalysisWidget::onColorPicked(const QColor& color,
                                   const QPoint& imagePos)
{
    if ( img.isNull() )
        return;

    const QRgb pix = color.rgb();

    selected.red   = qRed(pix);
    selected.green = qGreen(pix);
    selected.blue  = qBlue(pix);

    QPixmap pm(12,12);
    pm.fill(QColor(selected.red, selected.green, selected.blue));
    color_list->setItemIcon(ComboBoxColorIndex::SELECTED, pm);
    color_list->setCurrentIndex(ComboBoxColorIndex::SELECTED);

    refresh_rgb(ComboBoxColorIndex::SELECTED);
}

void AnalysisWidget::get_list_color()
{
    QColor color;
    QString title_str;

    if( id_this == 1 )
    {
        title_str = tr("Select list 1 color");
    }
    else if( id_this == 2 )
    {
        title_str = tr("Select list 2 color");
    }

    color = QColorDialog::getColor(Qt::white, this,
                                   title_str);

    if( color.isValid() )
    {
        selected.red   = (unsigned char)(color.red());
        selected.green = (unsigned char)(color.green());
        selected.blue  = (unsigned char)(color.blue());

        QPixmap pm(12,12);
        pm.fill(color);
        color_list->setItemIcon(ComboBoxColorIndex::SELECTED,pm);

        color_list->setCurrentIndex(ComboBoxColorIndex::SELECTED);

        refresh_rgb( int(ComboBoxColorIndex::SELECTED) );
    }
}

void AnalysisWidget::save_settings() const
{
    QSettings settings;

    settings.setValue("Main/Name/last_directory_used", last_directory_used);

    settings.setValue( "Analysis/combo"+QString::number(id_this),
                       color_list->currentIndex() );

    settings.setValue( "Analysis/R"+QString::number(id_this),
                       int(selected.red) );
    settings.setValue( "Analysis/G"+QString::number(id_this),
                       int(selected.green) );
    settings.setValue( "Analysis/B"+ QString::number(id_this),
                       int(selected.blue) );
}

}
