// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "about_window.hpp"
#include "application_settings.hpp"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImageReader>
#include <QImageWriter>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSysInfo>
#include <QTabWidget>
#include <QTextEdit>

namespace ofeli_app
{

AboutWindow::AboutWindow(QWidget* parent)
    : QDialog(parent)
{
    QSettings settings;

    setWindowTitle(tr("About Ofeli"));

    if (settings.contains("ui_geometry/about_window"))
    {
        restoreGeometry(settings.value("ui_geometry/about_window").toByteArray());
    }
    else
    {
        resize(700, 400);
    }

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
    license_window_->setLayout(layout_license);

    if (settings.contains("ui_geometry/license_window"))
    {
        license_window_->restoreGeometry(
            settings.value("ui_geometry/license_window").toByteArray());
    }
    else
    {
        license_window_->resize(500, 500);
    }

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

    QLabel* tech_label = new QLabel;
    tech_label->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);
    tech_label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse |
                                        Qt::LinksAccessibleByKeyboard);

    tech_label->setOpenExternalLinks(true);
    tech_label->setWordWrap(true);

    tech_label->setText(buildTechnicalSection());

    QVBoxLayout* tech_layout = new QVBoxLayout;
    tech_layout->addWidget(tech_label);

    QLabel* support_label = new QLabel;

    support_label->setText(
        tr("<p><b>Support Ofeli</b></p>"

           "<p>Ofeli is developed and maintained as an independent "
           "research-oriented project.</p>"

           "<p>If you find the software useful for research, teaching, "
           "or experimentation, you may support its continued development "
           "through a voluntary donation.</p>"

           "<p>Your support helps sustain long-term maintenance, "
           "improvements, and future extensions of the project.</p>"

           "<p><i>No features are restricted. Donations are entirely optional.</i></p>"));

    QPushButton* donate = new QPushButton(tr("Donate via PayPal"));
    donate->setAutoDefault(false);
    donate->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);

    connect(donate, &QPushButton::clicked, this,
            []()
            {
                QDesktopServices::openUrl(
                    QUrl("https://www.paypal.me/fabienbessy", QUrl::TolerantMode));
            });

    QVBoxLayout* support_layout = new QVBoxLayout;
    support_layout->addStretch();
    support_layout->addWidget(support_label);
    support_layout->addSpacing(16);
    support_layout->addWidget(donate);
    support_layout->addStretch();

    QWidget* page1 = new QWidget;
    QWidget* page2 = new QWidget;
    QWidget* page3 = new QWidget;
    QWidget* page4 = new QWidget;
    QWidget* page5 = new QWidget;
    page1->setLayout(overview_layout);
    page2->setLayout(scientific_layout);
    page3->setLayout(author_layout);
    page4->setLayout(tech_layout);
    page5->setLayout(support_layout);

    QTabWidget* tabs = new QTabWidget;
    tabs->addTab(page1, tr("Overview"));
    tabs->addTab(page2, tr("Scientific"));
    tabs->addTab(page3, tr("Author"));
    tabs->addTab(page4, tr("Technical"));
    tabs->addTab(page5, tr("Support"));

    ///////////////////////////////////////
    ///////////////////////////////////////
    ///////////////////////////////////////

    QHBoxLayout* layout_this = new QHBoxLayout;
    layout_this->addLayout(left_layout);
    layout_this->addWidget(tabs);
    layout_this->setSizeConstraint(QLayout::SetMinimumSize);

    this->setLayout(layout_this);
}

QString buildTechnicalSection()
{
    QStringList readFormats;
    for (const QByteArray& f : QImageReader::supportedImageFormats())
        readFormats << QString::fromLatin1(f).toUpper();

    QStringList writeFormats;
    for (const QByteArray& f : QImageWriter::supportedImageFormats())
        writeFormats << QString::fromLatin1(f).toUpper();

    readFormats.sort();
    writeFormats.sort();

    QSettings s;
    QFileInfo info(s.fileName());
    QString configDir = info.dir().absolutePath();

    QString html;

    html += "<p style='font-size:14pt; font-weight:bold; margin:0;'>"
            "Technical information"
            "</p><br>";

    html += "<b>Configuration directory</b><br>";
    html += "<pre style='margin:0;'>" + configDir + "</pre><br>";

    html += "<b>Image formats (Qt plugins)</b><br>";
    html += "<span style='font-size:9pt;'>"
            "Image format support is provided by Qt image format plugins.<br>"
            "Available formats depend on the Qt installation and platform."
            "</span><br><br>";

    html += "<pre style='margin:0;'>";
    html += "Read  : " + readFormats.join(", ") + "\n";
    html += "Write : " + writeFormats.join(", ");
    html += "</pre>";

    html += "<br><b>Build information</b><br>";
    html += "<pre style='margin:0;'>";

    html += "Qt (build)   : " + QString(QT_VERSION_STR) + "\n";
    html += "Qt (runtime) : " + QString(qVersion()) + "\n";
    html += "Architecture : " + QSysInfo::currentCpuArchitecture() + "\n";
    html += "OS           : " + QSysInfo::prettyProductName() + "\n";

#ifdef NDEBUG
    html += "Build type   : Release\n";
#else
    html += "Build type   : Debug\n";
#endif

    html += "</pre>";

    html += "<br><b>Toolchain</b><br>";
    html += "<pre style='margin:0;'>";

#if defined(__clang__)
    html += "Compiler : Clang ";
    html += __clang_version__;
    html += "\n";
#elif defined(__GNUC__)
    html += "Compiler : GCC ";
    html += __VERSION__;
    html += "\n";
#elif defined(_MSC_VER)
    html += "Compiler : MSVC ";
    html += QString::number(_MSC_VER);
    html += "\n";
#else
    html += "Compiler : Unknown\n";
#endif

    QString cppStandardPretty;

#if __cplusplus >= 202302L
    cppStandardPretty = "C++23";
#elif __cplusplus >= 202002L
    cppStandardPretty = "C++20";
#elif __cplusplus >= 201703L
    cppStandardPretty = "C++17";
#elif __cplusplus >= 201402L
    cppStandardPretty = "C++14";
#elif __cplusplus >= 201103L
    cppStandardPretty = "C++11";
#else
    cppStandardPretty = "Pre-C++11";
#endif

    html += "C++ standard : " + cppStandardPretty + "\n";

#ifdef OFELI_CMAKE_VERSION
    html += "CMake version : ";
    html += OFELI_CMAKE_VERSION;
    html += "\n";
#endif

#ifdef OFELI_CMAKE_GENERATOR
    html += "CMake generator : ";
    html += OFELI_CMAKE_GENERATOR;
    html += "\n";
#endif

#ifdef OFELI_CMAKE_BUILD_TYPE
    html += "CMake build type : ";
    html += OFELI_CMAKE_BUILD_TYPE;
    html += "\n";
#endif

    html += "</pre>";

    return html;
}

void AboutWindow::open_webpage()
{
    QDesktopServices::openUrl(
        QUrl("https://sourceforge.net/projects/fastlevelset/", QUrl::TolerantMode));
}

void AboutWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;

    settings.setValue("ui_geometry/about_window", saveGeometry());
    settings.setValue("ui_geometry/license_window", license_window_->saveGeometry());

    QDialog::closeEvent(event);
}

} // namespace ofeli_app
