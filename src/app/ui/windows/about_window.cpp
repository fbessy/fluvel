// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "about_window.hpp"
#include "application_settings.hpp"

#include <QCoreApplication>
#include <QCryptographicHash>
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

namespace
{

static QString packageType()
{
#ifdef OFELI_PACKAGE_TYPE
    return QString(OFELI_PACKAGE_TYPE);
#else
    if (qEnvironmentVariableIsSet("FLATPAK_ID"))
        return "Flatpak";
    return "Native";
#endif
}

static QString runtimeInfo()
{
    QString info;
    info += QSysInfo::prettyProductName() + "\n";
    info += "CPU: " + QSysInfo::currentCpuArchitecture();
    return info;
}

static QString compilerInfo()
{
#if defined(__clang__)
    return QString("Clang %1.%2.%3")
        .arg(__clang_major__)
        .arg(__clang_minor__)
        .arg(__clang_patchlevel__);
#elif defined(__GNUC__)
    return QString("GCC %1.%2.%3").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
    return QString("MSVC %1").arg(_MSC_VER);
#else
    return "Unknown compiler";
#endif
}

static QString buildFingerprint()
{
    QString data;

    data += "Version:" OFELI_VERSION "\n";
    data += "Git:" FLUVEL_GIT_COMMIT "\n";
    data += "BuildType:" FLUVEL_BUILD_TYPE "\n";
    data += "Qt:" + QString(qVersion()) + "\n";
    data += "Compiler:" + QString(__VERSION__) + "\n";
    data += "CMake:" FLUVEL_CMAKE_VERSION "\n";
    data += "Package:" + packageType() + "\n";
    data += "Runtime:" + runtimeInfo() + "\n";

    QByteArray hash = QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha256);

    return hash.toHex().left(16).toUpper();
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

    // =========================
    // Title
    // =========================

    html += "<div style='font-size:14pt; font-weight:bold; margin-bottom:12px;'>"
            "Technical information"
            "</div>";

    // =========================
    // Application
    // =========================

    html += "<div style='font-size:12pt; font-weight:bold; margin-top:8px;'>"
            "Application"
            "</div>";

    html += "<div style='margin-left:12px;'>";

    html += "<div style='margin-bottom:6px;'>"
            "<b>Version:</b> " OFELI_VERSION "</div>";

    html += "<div style='margin-bottom:4px;'><b>Configuration directory</b></div>";
    html += "<div style='font-family:monospace; margin-bottom:8px;'>" + configDir + "</div>";

    html += "<div style='margin-bottom:4px;'><b>Image formats</b></div>";

    html += "<div style='margin-left:12px; font-size:9pt; color:#444; margin-bottom:6px;'>"
            "Image format support is provided by Qt image format plugins.<br>"
            "Available formats depend on the Qt installation and platform."
            "</div>";

    html += "<div style='font-family:monospace; white-space:normal; word-wrap:break-word;'>";
    html += "<b>Read :</b> " + readFormats.join(", ") + "<br>";
    html += "<b>Write:</b> " + writeFormats.join(", ");
    html += "</div>";

    html += "</div>"; // end Application block

    // =========================
    // Environment
    // =========================

    html += "<div style='font-size:12pt; font-weight:bold; margin-top:14px;'>"
            "Environment"
            "</div>";

    html += "<div style='margin-left:12px; font-family:monospace; white-space:pre-wrap;'>" +
            runtimeInfo() +
            "<br>"
            "Qt (runtime): " +
            QString(qVersion()) + "</div>";

    // =========================
    // Package
    // =========================

    html += "<div style='font-size:12pt; font-weight:bold; margin-top:14px;'>"
            "Package"
            "</div>";

    html += "<div style='margin-left:12px; font-family:monospace;'>"
            "Type: " +
            packageType() + "</div>";

    // =========================
    // Build
    // =========================

    html += "<div style='font-size:12pt; font-weight:bold; margin-top:14px;'>"
            "Build"
            "</div>";

    html += "<div style='margin-left:12px; font-family:monospace; white-space:pre-wrap;'>"
            "Git commit: " FLUVEL_GIT_COMMIT "<br>"
            "Build type: " FLUVEL_BUILD_TYPE "<br>"
            "Qt (build): " +
            QString(QT_VERSION_STR) +
            "<br>"
            "Compiler: " +
            compilerInfo() +
            "<br>"
            "CMake: " FLUVEL_CMAKE_VERSION +
            "</div>";

    // =========================
    // Fingerprint
    // =========================

    html += "<div style='font-size:12pt; font-weight:bold; margin-top:14px;'>"
            "Build fingerprint"
            "</div>";

    html +=
        "<div style='margin-left:12px; font-family:monospace;'>" + buildFingerprint() + "</div>";

    return html;
}

} // namespace

AboutWindow::AboutWindow(QWidget* parent)
    : QDialog(parent)
{
    QSettings settings;

    setWindowTitle(tr("About Fluvel"));

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
    QIcon ofeli_icon(":/icons/app/fluvel.svg");
    icon_label->setPixmap(ofeli_icon.pixmap(48 * 3, 48 * 3));
    icon_label->setAlignment(Qt::AlignCenter);

    QLabel* name_label = new QLabel;
    name_label->setText("<b>Fluvel</b>");
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
        license_textedit->setText(QString::fromUtf8(file.readAll()));

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
        tr("<p><b>Fluvel</b> is a research-oriented image segmentation application "
           "designed for fast and efficient experimentation with level-set active contour "
           "methods.</p>"

           "<p>The software integrates real-time image processing, video stream handling, "
           "image pre-processing tools, and quantitative evaluation metrics such as "
           "Hausdorff distance for segmentation analysis.</p>"

           "<p>It provides an interactive environment for testing and analyzing "
           "discrete level-set implementations in both static images and dynamic video "
           "scenarios.</p>"

           "<p>Fluvel focuses on performance, reproducibility, and clarity, "
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

           "<p>Fluvel integrates several fundamental methods in image segmentation, "
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

    QTextEdit* tech_text = new QTextEdit;
    tech_text->setReadOnly(true);
    tech_text->setWordWrapMode(QTextOption::WordWrap);
    tech_text->setHtml(buildTechnicalSection());

    QVBoxLayout* tech_layout = new QVBoxLayout;
    tech_layout->addWidget(tech_text);

    QLabel* support_label = new QLabel;
    support_label->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);
    support_label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse |
                                           Qt::LinksAccessibleByKeyboard);

    support_label->setOpenExternalLinks(true);
    support_label->setWordWrap(true);

    support_label->setText(
        tr("<p><b>Support Fluvel</b></p>"

           "<p>Fluvel is developed and maintained as an independent "
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
