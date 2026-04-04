// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "analysis_widget.hpp"

#include "analysis_window.hpp"
#include "color_adapters.hpp"
#include "color_picker_behavior.hpp"
#include "drag_drop_behavior.hpp"
#include "image_viewer_widget.hpp"
#include "interaction_set.hpp"
#include "pan_behavior.hpp"

#include <QColorDialog>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

#include <random>

namespace fluvel_app
{

int AnalysisWidget::countThis = 0;

AnalysisWidget::AnalysisWidget(QWidget* parent)
    : QWidget(parent)
{
    ++countThis;         // static variable to count the instances
    idThis_ = countThis; // in order to know if *this is the first or the second widget of
                         // evaluation_window

    QSettings settings;

    textListLength_ = new QLabel(this);
    textListLength_->setAlignment(Qt::AlignCenter);
    if (idThis_ == 1)
    {
        textListLength_->setText("<font color=red>" + tr("List 1 length = ") +
                                 QString::number(shape_.points().size()));
    }
    else if (idThis_ == 2)
    {
        textListLength_->setText("<font color=red>" + tr("List 2 length = ") +
                                 QString::number(shape_.points().size()));
    }

    /////////////////////////////////////////////////////////////////////////////////

    nameLabel_ = new QLabel(this);
    nameLabel_->setText(tr("Title - Size"));
    nameLabel_->setAlignment(Qt::AlignCenter);

    ///////////////////////////////////////

    imageViewer_ = new ImageViewerWidget(this);
    auto interaction = std::make_unique<InteractionSet>();
    // interaction->addBehavior(std::make_unique<AutoFitBehavior>());
    // interaction->addBehavior(std::make_unique<FullscreenBehavior>());
    interaction->addBehavior(std::make_unique<PanBehavior>());
    interaction->addBehavior(std::make_unique<ColorPickerBehavior>());
    interaction->addBehavior(std::make_unique<DragDropBehavior>());
    imageViewer_->setInteraction(interaction.release());

    imageViewer_->setListener(this);

    ///////////////////////////////////////

    openButton_ = new QPushButton(tr("Open image") + " " + QString::number(idThis_));

    QVBoxLayout* img_layout = new QVBoxLayout;
    img_layout->addWidget(nameLabel_);
    img_layout->addWidget(imageViewer_);
    img_layout->addWidget(openButton_);
    QGroupBox* img_group = new QGroupBox(tr("Image") + " " + QString::number(idThis_));
    img_group->setLayout(img_layout);

    //////////////////////////////////////////////////////////////////////////////////

    colorList_ = new QComboBox;

    QPixmap pm(12, 12);

    pm.fill(Qt::red);
    colorList_->addItem(pm, tr("Red"));
    pm.fill(Qt::green);
    colorList_->addItem(pm, tr("Green"));
    pm.fill(Qt::blue);
    colorList_->addItem(pm, tr("Blue"));
    pm.fill(Qt::cyan);
    colorList_->addItem(pm, tr("Cyan"));
    pm.fill(Qt::magenta);
    colorList_->addItem(pm, tr("Magenta"));
    pm.fill(Qt::yellow);
    colorList_->addItem(pm, tr("Yellow"));
    pm.fill(Qt::black);
    colorList_->addItem(pm, tr("Black"));
    pm.fill(Qt::white);
    colorList_->addItem(pm, tr("White"));

    selected_.red = static_cast<unsigned char>(
        settings.value("Analysis/R" + QString::number(idThis_), 128).toInt());
    selected_.green = static_cast<unsigned char>(
        settings.value("Analysis/G" + QString::number(idThis_), 0).toInt());
    selected_.blue = static_cast<unsigned char>(
        settings.value("Analysis/B" + QString::number(idThis_), 255).toInt());

    pm.fill(toQColor(selected_));
    colorList_->addItem(pm, tr("Selected"));

    colorList_->setCurrentIndex(
        settings.value("Analysis/combo" + QString::number(idThis_), 0).toInt());

    ///////////////////////////////////////

    QPushButton* color_select = new QPushButton(tr("Select"));

    QFormLayout* form = new QFormLayout;
    form->addRow(tr("List from :"), colorList_);
    form->addRow(tr("<click on image> |"), color_select);

    QGroupBox* color_group = new QGroupBox(tr("Color") + " " + QString::number(idThis_));
    color_group->setLayout(form);

    noiseSp_ = new QSpinBox;
    noiseSp_->setSingleStep(1);
    noiseSp_->setMinimum(0);
    noiseSp_->setMaximum(100);
    noiseSp_->setSuffix(tr(" %"));
    noiseSp_->setValue(0);
    QFormLayout* noise_layout = new QFormLayout;
    noise_layout->addRow("noise =", noiseSp_);

    QVBoxLayout* this_layout = new QVBoxLayout;
    this_layout->addWidget(textListLength_);
    this_layout->addWidget(img_group);
    this_layout->addWidget(color_group);
    this_layout->addLayout(noise_layout);

    setLayout(this_layout);

    lastDirectoryUsed_ = settings.value("history/last_directory", QDir().homePath()).toString();

    nameFilters_ << "*.bmp"
                 //<< "*.dcm"
                 << "*.gif"
                 << "*.jpg" << "*.jpeg" << "*.mng"
                 << "*.pbm" << "*.png" << "*.pgm"
                 << "*.ppm" << "*.svg" << "*.svgz"
                 << "*.tiff" << "*.tif" << "*.xbm" << "*.xpm";

    nameFilters_.removeDuplicates();

    imageViewer_->setListener(this);

    connect(openButton_, &QPushButton::clicked, this, &AnalysisWidget::openFilename);

    connect(colorList_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &AnalysisWidget::refreshRgb);

    connect(color_select, &QPushButton::clicked, this, &AnalysisWidget::getListColor);

    auto* analysisWindow = qobject_cast<AnalysisWindow*>(parentWidget());
    Q_ASSERT(analysisWindow);

    connect(this, &AnalysisWidget::listChanged, analysisWindow, &AnalysisWindow::checkLists);

    connect(noiseSp_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &AnalysisWidget::refreshNoiseImage);
}

void AnalysisWidget::openFilename()
{
    absoluteName_ = QFileDialog::getOpenFileName(
        this, tr("Open File") + " " + QString::number(idThis_), lastDirectoryUsed_,
        tr("Image Files (%1)").arg(nameFilters_.join(" ")));
    openImage();
}

void AnalysisWidget::openImage()
{
    if (!absoluteName_.isEmpty())
    {
        image_ = QImage(absoluteName_);
        imageHeight_ = image_.height();
        imageWidth_ = image_.width();

        if (image_.isNull())
        {
            QMessageBox::information(
                this, tr("Opening error - Fluvel"),
                tr("Cannot load %1.").arg(QDir::toNativeSeparators(absoluteName_)));
            return;
        }

        refreshRgb(colorList_->currentIndex());

        QFileInfo fi(absoluteName_);
        QString name = fi.fileName();

        QString string_lists_text;
        string_lists_text = QString::number(imageWidth_) + "×" + QString::number(imageHeight_);
        nameLabel_->setText(name + " - " + string_lists_text);
    }
}

void AnalysisWidget::refreshRgb(int color_list_index)
{
    if (color_list_index == ComboBoxColorIndex::SELECTED)
        rgb_ = selected_;
    else
        rgb_ = get_color(color_list_index);

    refreshNoiseImage(noiseSp_->value());
}

void AnalysisWidget::createList()
{
    shape_.clear();

    QRgb pix;

    for (int y = 0; y < imageHeight_; ++y)
    {
        for (int x = 0; x < imageWidth_; ++x)
        {
            pix = noiseImage_.pixel(x, y);

            if (toRgb_uc(pix) == rgb_)
                shape_.pushBack(x, y);
        }
    }

    shape_.calculateCentroid();

    QString size_str = QString::number(shape_.points().size());

    QString color_str;
    if (shape_.points().empty())
        color_str = "<font color=red>";
    else
        color_str = "<font color=green>";

    QString list_str;
    if (idThis_ == 1)
        list_str = tr("List 1 length = ");
    else if (idThis_ == 2)
        list_str = tr("List 2 length = ");

    textListLength_->setText(color_str + list_str + size_str);

    emit listChanged();
}

void AnalysisWidget::refreshNoiseImage(int noise_percent)
{
    if (!image_.isNull())
    {
        noiseImage_ = image_;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::bernoulli_distribution proba_distri{float(noise_percent) / 100.f};

        QColor color(int(rgb_.red), int(rgb_.green), int(rgb_.blue));

        QRgb rgb_color = color.rgb();

        for (int y = 0; y < imageHeight_; ++y)
        {
            for (int x = 0; x < imageWidth_; ++x)
            {
                if (proba_distri(gen))
                    noiseImage_.setPixel(x, y, rgb_color);
            }
        }

        imageViewer_->setImage(noiseImage_);

        createList();
    }
}

void AnalysisWidget::onColorPicked(const QColor& color, const QPoint& /*imagePos*/)
{
    if (image_.isNull())
        return;

    selected_ = toRgb_uc(color);

    QPixmap pm(12, 12);
    pm.fill(toQColor(selected_));
    colorList_->setItemIcon(ComboBoxColorIndex::SELECTED, pm);
    colorList_->setCurrentIndex(ComboBoxColorIndex::SELECTED);

    refreshRgb(ComboBoxColorIndex::SELECTED);
}

void AnalysisWidget::getListColor()
{
    QColor color;
    QString title_str;

    if (idThis_ == 1)
        title_str = tr("Select list 1 color");
    else if (idThis_ == 2)
        title_str = tr("Select list 2 color");

    color = QColorDialog::getColor(Qt::white, this, title_str);

    if (color.isValid())
    {
        selected_ = toRgb_uc(color);

        QPixmap pm(12, 12);
        pm.fill(color);
        colorList_->setItemIcon(ComboBoxColorIndex::SELECTED, pm);

        colorList_->setCurrentIndex(ComboBoxColorIndex::SELECTED);

        refreshRgb(int(ComboBoxColorIndex::SELECTED));
    }
}

void AnalysisWidget::saveSettings() const
{
    QSettings settings;

    settings.setValue("history/last_directory", lastDirectoryUsed_);

    settings.setValue("Analysis/combo" + QString::number(idThis_), colorList_->currentIndex());

    settings.setValue("Analysis/R" + QString::number(idThis_), int(selected_.red));
    settings.setValue("Analysis/G" + QString::number(idThis_), int(selected_.green));
    settings.setValue("Analysis/B" + QString::number(idThis_), int(selected_.blue));
}

} // namespace fluvel_app
