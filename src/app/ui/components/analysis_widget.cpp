// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "analysis_widget.hpp"

#include "analysis_window.hpp"
#include "color_adapters.hpp"
#include "color_picker_behavior.hpp"
#include "interaction_set.hpp"
#include "pan_behavior.hpp"
#include "image_view.hpp"

#include <QtWidgets>

namespace ofeli_app
{

int AnalysisWidget::count_this = 0;

AnalysisWidget::AnalysisWidget(QWidget* parent)
    : QWidget(parent)
{
    ++count_this;          // static variable to count the instances
    id_this_ = count_this; // in order to know if *this is the first or the second widget of
                           // evaluation_window

    QSettings settings;

    text_list_length_ = new QLabel(this);
    text_list_length_->setAlignment(Qt::AlignCenter);
    if (id_this_ == 1)
    {
        text_list_length_->setText("<font color=red>" + tr("List 1 length = ") +
                                   QString::number(shape_.get_points().size()));
    }
    else if (id_this_ == 2)
    {
        text_list_length_->setText("<font color=red>" + tr("List 2 length = ") +
                                   QString::number(shape_.get_points().size()));
    }

    /////////////////////////////////////////////////////////////////////////////////

    name_label_ = new QLabel(this);
    name_label_->setText(tr("Title - Size"));
    name_label_->setAlignment(Qt::AlignCenter);

    ///////////////////////////////////////

    imageView_ = new ImageView(this);
    auto interaction = std::make_unique<InteractionSet>();
    // interaction->addBehavior(std::make_unique<AutoFitBehavior>());
    // interaction->addBehavior(std::make_unique<FullscreenBehavior>());
    interaction->addBehavior(std::make_unique<PanBehavior>());
    interaction->addBehavior(std::make_unique<ColorPickerBehavior>());
    imageView_->setInteraction(interaction.release());

    imageView_->setListener(this);

    ///////////////////////////////////////

    open_button_ = new QPushButton(tr("Open image") + " " + QString::number(id_this_));

    QVBoxLayout* img_layout = new QVBoxLayout;
    img_layout->addWidget(name_label_);
    img_layout->addWidget(imageView_);
    img_layout->addWidget(open_button_);
    QGroupBox* img_group = new QGroupBox(tr("Image") + " " + QString::number(id_this_));
    img_group->setLayout(img_layout);

    //////////////////////////////////////////////////////////////////////////////////

    color_list_ = new QComboBox;

    QPixmap pm(12, 12);

    pm.fill(Qt::red);
    color_list_->addItem(pm, tr("Red"));
    pm.fill(Qt::green);
    color_list_->addItem(pm, tr("Green"));
    pm.fill(Qt::blue);
    color_list_->addItem(pm, tr("Blue"));
    pm.fill(Qt::cyan);
    color_list_->addItem(pm, tr("Cyan"));
    pm.fill(Qt::magenta);
    color_list_->addItem(pm, tr("Magenta"));
    pm.fill(Qt::yellow);
    color_list_->addItem(pm, tr("Yellow"));
    pm.fill(Qt::black);
    color_list_->addItem(pm, tr("Black"));
    pm.fill(Qt::white);
    color_list_->addItem(pm, tr("White"));

    selected_.red = static_cast<unsigned char>(
        settings.value("Analysis/R" + QString::number(id_this_), 128).toInt());
    selected_.green = static_cast<unsigned char>(
        settings.value("Analysis/G" + QString::number(id_this_), 0).toInt());
    selected_.blue = static_cast<unsigned char>(
        settings.value("Analysis/B" + QString::number(id_this_), 255).toInt());

    pm.fill(toQColor(selected_));
    color_list_->addItem(pm, tr("Selected"));

    color_list_->setCurrentIndex(
        settings.value("Analysis/combo" + QString::number(id_this_), 0).toInt());

    ///////////////////////////////////////

    QPushButton* color_select = new QPushButton(tr("Select"));

    QFormLayout* form = new QFormLayout;
    form->addRow(tr("List from :"), color_list_);
    form->addRow(tr("<click on image> |"), color_select);

    QGroupBox* color_group = new QGroupBox(tr("Color") + " " + QString::number(id_this_));
    color_group->setLayout(form);

    noise_sp_ = new QSpinBox;
    noise_sp_->setSingleStep(1);
    noise_sp_->setMinimum(0);
    noise_sp_->setMaximum(100);
    noise_sp_->setSuffix(tr(" %"));
    noise_sp_->setValue(0);
    QFormLayout* noise_layout = new QFormLayout;
    noise_layout->addRow("noise =", noise_sp_);

    QVBoxLayout* this_layout = new QVBoxLayout;
    this_layout->addWidget(text_list_length_);
    this_layout->addWidget(img_group);
    this_layout->addWidget(color_group);
    this_layout->addLayout(noise_layout);

    setLayout(this_layout);

    last_directory_used_ =
        settings.value("Main/Name/last_directory_used", QDir().homePath()).toString();

    name_filters_ << "*.bmp"
                  //<< "*.dcm"
                  << "*.gif"
                  << "*.jpg" << "*.jpeg" << "*.mng"
                  << "*.pbm" << "*.png" << "*.pgm"
                  << "*.ppm" << "*.svg" << "*.svgz"
                  << "*.tiff" << "*.tif" << "*.xbm" << "*.xpm";

    name_filters_.removeDuplicates();

    imageView_->setListener(this);

    connect(open_button_, &QPushButton::clicked, this, &AnalysisWidget::open_filename);

    connect(color_list_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &AnalysisWidget::refresh_rgb);

    connect(color_select, &QPushButton::clicked, this, &AnalysisWidget::get_list_color);

    auto* analysisWindow = qobject_cast<AnalysisWindow*>(parentWidget());
    Q_ASSERT(analysisWindow);

    connect(this, &AnalysisWidget::change_list, analysisWindow, &AnalysisWindow::check_lists);

    connect(noise_sp_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &AnalysisWidget::refresh_img_noise);
}

void AnalysisWidget::open_filename()
{
    absolute_name_ = QFileDialog::getOpenFileName(
        this, tr("Open File") + " " + QString::number(id_this_), last_directory_used_,
        tr("Image Files (%1)").arg(name_filters_.join(" ")));
    open_img();
}

void AnalysisWidget::open_img()
{
    if (!absolute_name_.isEmpty())
    {
        img_ = QImage(absolute_name_);
        img_height_ = img_.height();
        img_width_ = img_.width();

        if (img_.isNull())
        {
            QMessageBox::information(
                this, tr("Opening error - Ofeli"),
                tr("Cannot load %1.").arg(QDir::toNativeSeparators(absolute_name_)));
            return;
        }

        refresh_rgb(color_list_->currentIndex());

        QFileInfo fi(absolute_name_);
        QString name = fi.fileName();

        QString string_lists_text;
        string_lists_text = QString::number(img_width_) + "×" + QString::number(img_height_);
        name_label_->setText(name + " - " + string_lists_text);
    }
}

void AnalysisWidget::refresh_rgb(int color_list_index)
{
    if (color_list_index == ComboBoxColorIndex::SELECTED)
    {
        rgb_ = selected_;
    }
    else
    {
        rgb_ = get_color(color_list_index);
    }

    refresh_img_noise(noise_sp_->value());
}

void AnalysisWidget::create_list()
{
    shape_.clear();

    QRgb pix;

    for (int y = 0; y < img_height_; ++y)
    {
        for (int x = 0; x < img_width_; ++x)
        {
            pix = img_noise_.pixel(x, y);

            if (toRgb_uc(pix) == rgb_)
            {
                shape_.push_back(x, y);
            }
        }
    }

    shape_.calculate_centroid();

    QString size_str = QString::number(shape_.get_points().size());

    QString color_str;
    if (shape_.get_points().empty())
    {
        color_str = "<font color=red>";
    }
    else
    {
        color_str = "<font color=green>";
    }

    QString list_str;
    if (id_this_ == 1)
    {
        list_str = tr("List 1 length = ");
    }
    else if (id_this_ == 2)
    {
        list_str = tr("List 2 length = ");
    }

    text_list_length_->setText(color_str + list_str + size_str);

    emit change_list();
}

void AnalysisWidget::refresh_img_noise(int noise_percent)
{
    if (!img_.isNull())
    {
        img_noise_ = img_;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::bernoulli_distribution proba_distri{float(noise_percent) / 100.f};

        QColor color(int(rgb_.red), int(rgb_.green), int(rgb_.blue));

        QRgb rgb_color = color.rgb();

        for (int y = 0; y < img_height_; ++y)
        {
            for (int x = 0; x < img_width_; ++x)
            {
                if (proba_distri(gen))
                {
                    img_noise_.setPixel(x, y, rgb_color);
                }
            }
        }

        imageView_->setImage(img_noise_);

        create_list();
    }
}

void AnalysisWidget::onColorPicked(const QColor& color, const QPoint& /*imagePos*/)
{
    if (img_.isNull())
        return;

    selected_ = toRgb_uc(color);

    QPixmap pm(12, 12);
    pm.fill(toQColor(selected_));
    color_list_->setItemIcon(ComboBoxColorIndex::SELECTED, pm);
    color_list_->setCurrentIndex(ComboBoxColorIndex::SELECTED);

    refresh_rgb(ComboBoxColorIndex::SELECTED);
}

void AnalysisWidget::get_list_color()
{
    QColor color;
    QString title_str;

    if (id_this_ == 1)
    {
        title_str = tr("Select list 1 color");
    }
    else if (id_this_ == 2)
    {
        title_str = tr("Select list 2 color");
    }

    color = QColorDialog::getColor(Qt::white, this, title_str);

    if (color.isValid())
    {
        selected_ = toRgb_uc(color);

        QPixmap pm(12, 12);
        pm.fill(color);
        color_list_->setItemIcon(ComboBoxColorIndex::SELECTED, pm);

        color_list_->setCurrentIndex(ComboBoxColorIndex::SELECTED);

        refresh_rgb(int(ComboBoxColorIndex::SELECTED));
    }
}

void AnalysisWidget::save_settings() const
{
    QSettings settings;

    settings.setValue("Main/Name/last_directory_used", last_directory_used_);

    settings.setValue("Analysis/combo" + QString::number(id_this_), color_list_->currentIndex());

    settings.setValue("Analysis/R" + QString::number(id_this_), int(selected_.red));
    settings.setValue("Analysis/G" + QString::number(id_this_), int(selected_.green));
    settings.setValue("Analysis/B" + QString::number(id_this_), int(selected_.blue));
}

} // namespace ofeli_app
