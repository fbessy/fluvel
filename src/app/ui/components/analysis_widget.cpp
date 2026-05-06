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
        textListLength_->setText("<font color=red>" + tr("List 1 size: ") +
                                 QString::number(shape_.points().size()));
    }
    else if (idThis_ == 2)
    {
        textListLength_->setText("<font color=red>" + tr("List 2 size: ") +
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

    selectedColor_.red = static_cast<unsigned char>(
        settings.value("Analysis/R" + QString::number(idThis_), 128).toInt());
    selectedColor_.green = static_cast<unsigned char>(
        settings.value("Analysis/G" + QString::number(idThis_), 0).toInt());
    selectedColor_.blue = static_cast<unsigned char>(
        settings.value("Analysis/B" + QString::number(idThis_), 255).toInt());

    QColor initialColor = toQColor(selectedColor_);
    colorSelector_ = new ColorSelectorWidget(this, initialColor);

    QFormLayout* form = new QFormLayout;
    form->addRow(tr("Color: "), colorSelector_);

    QGroupBox* color_group = new QGroupBox(tr("Color") + " " + QString::number(idThis_));
    color_group->setLayout(form);

    ///////////////////////////////////////

    noiseSp_ = new QSpinBox;
    noiseSp_->setSingleStep(1);
    noiseSp_->setMinimum(0);
    noiseSp_->setMaximum(100);
    noiseSp_->setSuffix(" %");
    noiseSp_->setValue(0);

    noiseGroup_ = new QGroupBox(tr("Noise"));
    noiseGroup_->setCheckable(true);
    noiseGroup_->setChecked(false);

    QFormLayout* noiseLayout = new QFormLayout;
    noiseLayout->addRow(tr("Amount:"), noiseSp_);

    noiseGroup_->setLayout(noiseLayout);

    QVBoxLayout* this_layout = new QVBoxLayout;
    this_layout->addWidget(textListLength_);
    this_layout->addWidget(img_group);
    this_layout->addWidget(color_group);
    this_layout->addWidget(noiseGroup_);

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

    connect(openButton_, &QPushButton::clicked, this, &AnalysisWidget::openFilename);

    connect(colorSelector_, &ColorSelectorWidget::colorSelected, this,
            &AnalysisWidget::onColorSelected);

    auto* analysisWindow = qobject_cast<AnalysisWindow*>(parentWidget());
    Q_ASSERT(analysisWindow);

    connect(this, &AnalysisWidget::listChanged, analysisWindow, &AnalysisWindow::checkLists);

    connect(noiseGroup_, &QGroupBox::toggled, this,
            [this](bool)
            {
                refreshWithNoise(noiseSp_->value());
            });

    connect(noiseSp_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &AnalysisWidget::refreshWithNoise);

    refresh();
}

void AnalysisWidget::onColorSelected(const QColor& c)
{
    selectedColor_ = toRgb_uc(c);
    refresh();
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

        refresh();

        QFileInfo fi(absoluteName_);
        QString name = fi.fileName();

        QString string_lists_text;
        string_lists_text = QString::number(imageWidth_) + "×" + QString::number(imageHeight_);
        nameLabel_->setText(name + " - " + string_lists_text);
    }
}

void AnalysisWidget::refresh()
{
    refreshWithNoise(noiseSp_->value());
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

            if (toRgb_uc(pix) == selectedColor_)
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
        list_str = tr("List 1 size: ");
    else if (idThis_ == 2)
        list_str = tr("List 2 size: ");

    textListLength_->setText(color_str + list_str + size_str);

    emit listChanged();
}

void AnalysisWidget::refreshWithNoise(int noisePercent)
{
    if (image_.isNull())
        return;

    noiseImage_ = image_;

    // 🔥 si désactivé → early return propre
    if (!noiseGroup_->isChecked() || noisePercent == 0)
    {
        imageViewer_->setImage(noiseImage_);
        createList();
        return;
    }

    std::bernoulli_distribution proba_distri{float(noisePercent) / 100.f};

    QRgb rgb_color =
        QColor(int(selectedColor_.red), int(selectedColor_.green), int(selectedColor_.blue)).rgb();

    for (int y = 0; y < imageHeight_; ++y)
    {
        for (int x = 0; x < imageWidth_; ++x)
        {
            if (proba_distri(rng_))
                noiseImage_.setPixel(x, y, rgb_color);
        }
    }

    imageViewer_->setImage(noiseImage_);
    createList();
}

void AnalysisWidget::onColorPicked(const QColor& color, const QPoint&)
{
    if (image_.isNull())
        return;

    colorSelector_->setSelectedColor(color);
}

void AnalysisWidget::saveSettings() const
{
    QSettings settings;

    settings.setValue("history/last_directory", lastDirectoryUsed_);

    // settings.setValue("Analysis/combo" + QString::number(idThis_), colorList_->currentIndex());

    settings.setValue("Analysis/R" + QString::number(idThis_), int(selectedColor_.red));
    settings.setValue("Analysis/G" + QString::number(idThis_), int(selectedColor_.green));
    settings.setValue("Analysis/B" + QString::number(idThis_), int(selectedColor_.blue));
}

} // namespace fluvel_app
