// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "about_window.hpp"
#include "application_settings.hpp"

#include <QtWidgets>

namespace ofeli_app
{

AboutWindow::AboutWindow(QWidget* parent)
    : QDialog(parent)
{
    QSettings settings;

    setWindowTitle(tr("About Ofeli"));

    const auto geo_about = settings.value("About/Window/geometry").toByteArray();
    if (!geo_about.isEmpty())
        restoreGeometry(geo_about);

    ///////////////////////////////////////
    //////         Left Part        ///////
    ///////////////////////////////////////

    QLabel* icon_label = new QLabel;
    QIcon ofeli_icon(":/icons/app/Ofeli.svg");
    icon_label->setPixmap(ofeli_icon.pixmap(48 * 3, 48 * 3));
    icon_label->setAlignment(Qt::AlignCenter);

    QLabel* name_label = new QLabel;
    name_label->setText("<b>Ofeli</b>");
    QFont font1;
    font1.setPointSize(22);
    font1.setBold(true);
    name_label->setFont(font1);
    name_label->setAlignment(Qt::AlignCenter);
    name_label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse |
                                        Qt::LinksAccessibleByKeyboard);

    QLabel* version_label = new QLabel;
    QFont font2;
    font2.setPointSize(12);
    version_label->setFont(font2);
    version_label->setText("Version 2.0.0");
    version_label->setAlignment(Qt::AlignCenter);
    version_label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse |
                                           Qt::LinksAccessibleByKeyboard);

    QLabel* years_label = new QLabel;
    QFont font3;
    font3.setPointSize(9);
    years_label->setFont(font3);
    years_label->setText("© 2010–2015, 2024–2026"
                         "\nFabien Bessy"
                         "\nLicensed under CeCILL v2.1");
    years_label->setAlignment(Qt::AlignCenter);
    years_label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse |
                                         Qt::LinksAccessibleByKeyboard);

    QPushButton* webpage = new QPushButton(tr("Web page"));
    webpage->setAutoDefault(false);

    connect(webpage, &QPushButton::clicked, this, &AboutWindow::open_webpage);

    QPushButton* license = new QPushButton(tr("License"));
    license->setAutoDefault(false);

    QString locale = QLocale::system().name().section('_', 0, 0);

    const auto& config = AppSettings::instance();
    const auto language = config.app_language;

    QString txt_file;
    if (language == Language::French || (language == Language::System && locale == "fr"))
    {
        txt_file = QString(":/licenses/Licence_CeCILL_V2.1-fr.txt");
    }
    else
    {
        txt_file = QString(":/licenses/Licence_CeCILL_V2.1-en.txt");
    }

    QFile file(txt_file);
    QTextEdit* license_textedit = new QTextEdit;
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream ts(&file);
        // ts.setCodec("ISO 8859-1");
        //  Qt6
        ts.setEncoding(QStringConverter::Latin1);
        license_textedit->setText(ts.readAll());
    }

    license_textedit->setReadOnly(true);

    QHBoxLayout* layout_license = new QHBoxLayout;
    layout_license->addWidget(license_textedit);

    license_window_ = new QDialog(this);
    license_window_->setWindowTitle(tr("License"));

    const auto geo_license = settings.value("About/License/geometry").toByteArray();
    if (!geo_license.isEmpty())
        license_window_->restoreGeometry(geo_license);

    license_window_->setLayout(layout_license);

    connect(license, &QPushButton::clicked, license_window_, &QDialog::show);

    QVBoxLayout* left_layout = new QVBoxLayout;
    left_layout->addWidget(icon_label);
    left_layout->addWidget(name_label);
    left_layout->addWidget(version_label);
    left_layout->addWidget(years_label);
    left_layout->addSpacing(10);
    left_layout->addWidget(webpage);
    left_layout->addWidget(license);
    left_layout->addStretch(1);

    ///////////////////////////////////////
    ///////////////////////////////////////
    ///////////////////////////////////////

    ///////////////////////////////////////
    //////         Right Part        //////
    ///////////////////////////////////////

    QLabel* overview_label = new QLabel;
    overview_label->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);
    overview_label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse |
                                            Qt::LinksAccessibleByKeyboard);

    overview_label->setOpenExternalLinks(true);
    overview_label->setWordWrap(true);

    overview_label->setText(
        tr("<p><b>Ofeli</b> is a research-oriented image segmentation application "
           "designed for fast and efficient experimentation with level-set active contour "
           "methods.</p>"

           "<p>The software integrates real-time image processing, video stream handling, "
           "image pre-processing tools, and quantitative evaluation metrics such as "
           "Hausdorff distance for segmentation analysis.</p>"

           "<p>It provides an interactive environment for testing and analyzing "
           "discrete level-set implementations in both static images and dynamic video "
           "scenarios.</p>"

           "<p>Ofeli focuses on performance, reproducibility, and clarity, "
           "making it suitable for research, teaching, and experimental validation.</p>"));

    QVBoxLayout* overview_layout = new QVBoxLayout;
    overview_layout->addWidget(overview_label);

    QLabel* scientific_label = new QLabel;
    scientific_label->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);
    scientific_label->setTextInteractionFlags(
        Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);

    scientific_label->setOpenExternalLinks(true);
    scientific_label->setWordWrap(true);

    scientific_label->setText(
        tr("<p><b>Scientific Background</b></p>"

           "<p>Ofeli integrates several fundamental methods in image segmentation, "
           "including discrete level-set approximation (Shi & Karl, 2008), "
           "region-based active contours (Chan & Vese, 2001), "
           "anisotropic diffusion filtering (Perona & Malik, 1990), "
           "and quantitative evaluation using the Hausdorff distance.</p>"

           "<p>These methods provide a balance between computational efficiency, "
           "topological flexibility, and rigorous segmentation assessment.</p>"));

    QVBoxLayout* scientific_layout = new QVBoxLayout;
    scientific_layout->addWidget(scientific_label);

    QLabel* author_label = new QLabel;
    author_label->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);
    author_label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse |
                                          Qt::LinksAccessibleByKeyboard);

    author_label->setOpenExternalLinks(true);
    author_label->setWordWrap(true);

    author_label->setText(
        tr("<p><b>Author</b></p>"

           "<p>Fabien Bessy</p>"

           "<p>Originally developed in 2010 within the Pattern Recognition and "
           "Image Analysis research team, Laboratory of Computer Science (LIFAT), "
           "François Rabelais University, Tours, "
           "as part of the MSc in Medical Imaging.</p>"

           "<p>The project has been modernized and extended in 2024–2026.</p>"

           "<p>Contact: "
           "<a href='mailto:fabien.bessy@gmail.com'>fabien.bessy@gmail.com</a></p>"));

    QVBoxLayout* author_layout = new QVBoxLayout;
    author_layout->addWidget(author_label);

    QWidget* page1 = new QWidget;
    QWidget* page2 = new QWidget;
    QWidget* page3 = new QWidget;
    page1->setLayout(overview_layout);
    page2->setLayout(scientific_layout);
    page3->setLayout(author_layout);

    QTabWidget* tabs = new QTabWidget;
    tabs->addTab(page1, tr("Overview"));
    tabs->addTab(page2, tr("Scientific"));
    tabs->addTab(page3, tr("Author"));

    ///////////////////////////////////////
    ///////////////////////////////////////
    ///////////////////////////////////////

    QHBoxLayout* layout_this = new QHBoxLayout;
    layout_this->addLayout(left_layout);
    layout_this->addWidget(tabs);
    layout_this->setSizeConstraint(QLayout::SetMinimumSize);

    this->setLayout(layout_this);
}

void AboutWindow::open_webpage()
{
    QDesktopServices::openUrl(
        QUrl("https://sourceforge.net/projects/fastlevelset/", QUrl::TolerantMode));
}

void AboutWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;

    settings.setValue("About/Window/geometry", saveGeometry());
    settings.setValue("About/License/geometry", license_window_->saveGeometry());

    QDialog::closeEvent(event);
}

} // namespace ofeli_app
