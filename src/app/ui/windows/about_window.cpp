// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "about_window.hpp"
#include "application_settings.hpp"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QImageReader>
#include <QImageWriter>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QSettings>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimeZone>

#include <QtGlobal>

namespace fluvel_app
{

namespace
{

constexpr auto kProjectUrl = "https://fbessy.github.io/fluvel/";

QString buildVersionString()
{
    QString version = FLUVEL_VERSION;

#ifdef FLUVEL_BUILD_VERSION
    QString buildVersion = FLUVEL_BUILD_VERSION;

    // évite doublon si tag = version
    if (buildVersion != version)
    {
        version += QString(" (%1)").arg(buildVersion);
    }
#endif

    return version;
}

QString buildType =
#ifdef QT_DEBUG
    "Debug";
#else
    "Release";
#endif

QString packageType()
{
#ifdef FLUVEL_PACKAGE_TYPE
    return QString(FLUVEL_PACKAGE_TYPE);
#else
    if (qEnvironmentVariableIsSet("FLATPAK_ID"))
        return "Flatpak";

    if (qEnvironmentVariableIsSet("APPIMAGE"))
        return "AppImage";

    return "Native";
#endif
}

QString runtimeInfo()
{
    QString info;
    info += QSysInfo::prettyProductName() + "\n";
    info += "CPU: " + QSysInfo::currentCpuArchitecture();
    return info;
}

QString compilerInfo()
{
#if defined(__clang__)
    return QString("Clang %1.%2.%3")
        .arg(__clang_major__)
        .arg(__clang_minor__)
        .arg(__clang_patchlevel__);
#elif defined(__GNUC__)
    return QString("GCC %1.%2.%3").arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
    return QString("MSVC %1 (%2)").arg(_MSC_VER).arg(_MSC_FULL_VER);
#else
    return "Unknown compiler";
#endif
}

#include <QDateTime>
#include <QLocale>
#include <QTimeZone>

QString formattedBuildDate()
{
    QString raw = FLUVEL_BUILD_TIMESTAMP;

    QDateTime dt = QDateTime::fromString(raw, "yyyy-MM-dd HH:mm:ss");

    if (!dt.isValid())
        return "Invalid date";

    // 👉 on force UTC
    dt.setTimeZone(QTimeZone::utc());

    // 👉 format fixe en-GB
    return QLocale(QLocale::English, QLocale::UnitedKingdom)
        .toString(dt, "dd MMM yyyy, HH:mm:ss 'UTC'");
}

QString buildFingerprint()
{
    QString data;

    data += "Version:" FLUVEL_VERSION "\n";
    data += "Git:" FLUVEL_GIT_COMMIT "\n";
    data += QString("BuildType:%1\n").arg(buildType);
    data += "Qt:" + QString(qVersion()) + "\n";
    data += "Compiler:" + compilerInfo() + "\n";
    data += "CMake:" FLUVEL_CMAKE_VERSION "\n";
    data += "Package:" + packageType() + "\n";
    data += "Runtime:" + runtimeInfo() + "\n";
    data += "OS:" + QSysInfo::prettyProductName() + "\n";

    QByteArray hash = QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha256);

    return hash.toHex().left(16).toUpper();
}

} // namespace

AboutWindow::AboutWindow(QWidget* parent)
    : QDialog(parent)
{
    QSettings settings;

    setWindowTitle(tr("About Fluvel"));

    if (settings.contains("ui_geometry/about_window"))
        restoreGeometry(settings.value("ui_geometry/about_window").toByteArray());
    else
        resize(700, 400);

    ///////////////////////////////////////
    //////         Left Part        ///////
    ///////////////////////////////////////

    QLabel* iconLabel = new QLabel;
    QIcon fluvelIcon(":/icons/app/fluvel.svg");
    iconLabel->setPixmap(fluvelIcon.pixmap(48 * 3, 48 * 3));
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel* nameLabel = new QLabel;
    nameLabel->setText("<b>Fluvel</b>");
    QFont font1;
    font1.setPointSize(22);
    font1.setBold(true);
    nameLabel->setFont(font1);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse |
                                       Qt::LinksAccessibleByKeyboard);

    QLabel* versionLabel = new QLabel;
    QFont font2;
    font2.setPointSize(12);
    versionLabel->setFont(font2);

    QString shortVersion = FLUVEL_VERSION;

#ifdef FLUVEL_BUILD_VERSION
    if (QString(FLUVEL_BUILD_VERSION).startsWith("dev-"))
        shortVersion += "-dev";
#endif

    const QString verStr = QString(tr("Version ")) + shortVersion;

    versionLabel->setText(verStr);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse |
                                          Qt::LinksAccessibleByKeyboard);

    QLabel* yearsLabel = new QLabel;
    QFont font3;
    font3.setPointSize(9);
    yearsLabel->setFont(font3);
    yearsLabel->setText("© 2010–2015, 2024–2026"
                        "\nFabien Bessy"
                        "\nLicensed under CeCILL v2.1");
    yearsLabel->setAlignment(Qt::AlignCenter);
    yearsLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse |
                                        Qt::LinksAccessibleByKeyboard);

    QPushButton* license = new QPushButton(tr("License"));
    license->setAutoDefault(false);

    QString locale = QLocale::system().name().section('_', 0, 0);

    const auto& config = ApplicationSettings::instance();
    const auto language = config.appLanguage();

    QString txtFile;
    if (language == Language::French || (language == Language::System && locale == "fr"))
        txtFile = QString(":/licenses/Licence_CeCILL_V2.1-fr.txt");
    else
        txtFile = QString(":/licenses/Licence_CeCILL_V2.1-en.txt");

    QFile file(txtFile);
    QTextEdit* licenseTextedit = new QTextEdit;

    if (file.open(QIODevice::ReadOnly))
        licenseTextedit->setText(QString::fromUtf8(file.readAll()));

    licenseTextedit->setReadOnly(true);

    QHBoxLayout* layoutLicense = new QHBoxLayout;
    layoutLicense->addWidget(licenseTextedit);

    licenseWindow_ = new QDialog(this);
    licenseWindow_->setWindowTitle(tr("License"));
    licenseWindow_->setLayout(layoutLicense);

    if (settings.contains("ui_geometry/license_window"))
    {
        licenseWindow_->restoreGeometry(
            settings.value("ui_geometry/license_window").toByteArray());
    }
    else
    {
        licenseWindow_->resize(500, 500);
    }

    connect(license, &QPushButton::clicked, licenseWindow_, &QDialog::show);

    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->addWidget(iconLabel);
    leftLayout->addWidget(nameLabel);
    leftLayout->addWidget(versionLabel);
    leftLayout->addWidget(yearsLabel);
    leftLayout->addSpacing(10);
    leftLayout->addWidget(license);

    QLabel* linksLabel = new QLabel;

    linksLabel->setText(QString("<a href='https://fabienip.gitlab.io/fluvel/'>%1</a><br>"
                                "<a href='https://github.com/fbessy/fluvel/'>GitHub</a><br>"
                                "<a href='https://fbessy.github.io/fluvel/'>%2</a>")
                            .arg(tr("Documentation"))
                            .arg(tr("Home")));

    linksLabel->setOpenExternalLinks(true);

    linksLabel->setAlignment(Qt::AlignCenter);

    linksLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    leftLayout->addSpacing(6);
    leftLayout->addWidget(linksLabel);

    leftLayout->addStretch(1);

    ///////////////////////////////////////
    ///////////////////////////////////////
    ///////////////////////////////////////

    ///////////////////////////////////////
    //////         Right Part        //////
    ///////////////////////////////////////

    QLabel* overviewLabel = new QLabel;
    overviewLabel->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);
    overviewLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse |
                                           Qt::LinksAccessibleByKeyboard);

    overviewLabel->setOpenExternalLinks(true);
    overviewLabel->setWordWrap(true);

    overviewLabel->setText(
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

    QVBoxLayout* overviewLayout = new QVBoxLayout;
    overviewLayout->addWidget(overviewLabel);

    QLabel* scientificLabel = new QLabel;
    scientificLabel->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);
    scientificLabel->setTextInteractionFlags(
        Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);

    scientificLabel->setOpenExternalLinks(true);
    scientificLabel->setWordWrap(true);

    scientificLabel->setText(
        tr("<p><b>Scientific Background</b></p>"

           "<p>Fluvel integrates several fundamental methods in image segmentation, "
           "including discrete level-set approximation (Shi & Karl, 2008), "
           "region-based active contours (Chan & Vese, 2001), "
           "anisotropic diffusion filtering (Perona & Malik, 1990), "
           "and quantitative evaluation using the Hausdorff distance.</p>"

           "<p>These methods provide a balance between computational efficiency, "
           "topological flexibility, and rigorous segmentation assessment.</p>"));

    QVBoxLayout* scientificLayout = new QVBoxLayout;
    scientificLayout->addWidget(scientificLabel);

    QLabel* authorLabel = new QLabel;
    authorLabel->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);
    authorLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse |
                                         Qt::LinksAccessibleByKeyboard);

    authorLabel->setOpenExternalLinks(true);
    authorLabel->setWordWrap(true);

    authorLabel->setText(
        tr("<p><b>Fabien Bessy</b></p>"

           "<p>This project was originally developed in 2010 within the Pattern Recognition and "
           "Image Analysis research team, Laboratory of Computer Science (LIFAT), "
           "François Rabelais University, Tours, "
           "as part of the MSc in Medical Imaging.</p>"

           "<p>It has been modernized and extended in 2024–2026.</p>"

           "<p>Contact: "
           "<a href='mailto:fabien.bessy@gmail.com'>fabien.bessy@gmail.com</a></p>"));

    QVBoxLayout* authorLayout = new QVBoxLayout;
    authorLayout->addWidget(authorLabel);

    QTextEdit* techText = new QTextEdit;
    techText->setReadOnly(true);
    techText->setWordWrapMode(QTextOption::WordWrap);
    techText->setHtml(buildTechnicalSection());

    QVBoxLayout* techLayout = new QVBoxLayout;
    techLayout->addWidget(techText);

    QLabel* supportLabel = new QLabel;
    supportLabel->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);
    supportLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse |
                                          Qt::LinksAccessibleByKeyboard);

    supportLabel->setOpenExternalLinks(true);
    supportLabel->setWordWrap(true);

    supportLabel->setText(
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

    QVBoxLayout* supportLayout = new QVBoxLayout;
    supportLayout->addStretch();
    supportLayout->addWidget(supportLabel);
    supportLayout->addSpacing(16);
    supportLayout->addWidget(donate);
    supportLayout->addStretch();

    QWidget* page1 = new QWidget;
    QWidget* page2 = new QWidget;
    QWidget* page3 = new QWidget;
    QWidget* page4 = new QWidget;
    QWidget* page5 = new QWidget;
    page1->setLayout(overviewLayout);
    page2->setLayout(scientificLayout);
    page3->setLayout(authorLayout);
    page4->setLayout(techLayout);
    page5->setLayout(supportLayout);

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
    layout_this->addLayout(leftLayout);
    layout_this->addWidget(tabs);
    layout_this->setSizeConstraint(QLayout::SetMinimumSize);

    this->setLayout(layout_this);
}

void AboutWindow::openHomepage()
{
    QDesktopServices::openUrl(QUrl(kProjectUrl, QUrl::TolerantMode));
}

QString AboutWindow::buildTechnicalSection()
{
    const auto readFormatsRaw = QImageReader::supportedImageFormats();
    QStringList readFormats;
    readFormats.reserve(readFormatsRaw.size());

    for (const auto& f : readFormatsRaw)
        readFormats << QString::fromLatin1(f).toUpper();

    const auto writeFormatsRaw = QImageWriter::supportedImageFormats();
    QStringList writeFormats;
    writeFormats.reserve(writeFormatsRaw.size());

    for (const auto& f : writeFormatsRaw)
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

    html += "<div style='font-size:14pt; font-weight:bold; margin-bottom:12px;'>" +
            tr("Technical information") + "</div>";

    // =========================
    // Application
    // =========================

    html += "<div style='font-size:12pt; font-weight:bold; margin-top:8px;'>" + tr("Application") +
            "</div>";

    html += "<div style='margin-left:12px;'>";

    html += "<div style='margin-bottom:6px;'>"
            "<b>" +
            tr("Version") + ":</b> " + buildVersionString() + "</div>";

    html += "<div style='margin-bottom:4px;'><b>" + tr("Configuration directory") + "</b></div>";

    html += "<div style='font-family:monospace; margin-bottom:8px;'>" + configDir + "</div>";

    html += "<div style='margin-bottom:4px;'><b>" + tr("Image formats") + "</b></div>";

    html += "<div style='margin-left:12px; font-size:9pt; color:#444; margin-bottom:6px;'>";
    html += tr("Image format support is provided by Qt image format plugins.") + "<br>";
    html += tr("Available formats depend on the Qt installation and platform.");
    html += "</div>";

    html += "<div style='font-family:monospace; white-space:normal; word-wrap:break-word;'>";
    html += "<b>" + tr("Read") + " :</b> " + readFormats.join(", ") + "<br>";
    html += "<b>" + tr("Write") + ":</b> " + writeFormats.join(", ");
    html += "</div>";

    html += "</div>"; // end Application block

    // =========================
    // Environment
    // =========================

    html += "<div style='font-size:12pt; font-weight:bold; margin-top:14px;'>" + tr("Environment") +
            "</div>";

    html += "<div style='margin-left:12px; font-family:monospace; white-space:pre-wrap;'>" +
            runtimeInfo() + "<br>" + tr("Qt (runtime)") + ": " + QString(qVersion()) + "</div>";

    // =========================
    // Package
    // =========================

    html += "<div style='font-size:12pt; font-weight:bold; margin-top:14px;'>" + tr("Package") +
            "</div>";

    html += "<div style='margin-left:12px; font-family:monospace;'>" + tr("Type") + ": " +
            packageType() + "</div>";

    // =========================
    // Build
    // =========================

    html +=
        "<div style='font-size:12pt; font-weight:bold; margin-top:14px;'>" + tr("Build") + "</div>";

    auto addLine = [&](const QString& label, const QString& value)
    {
        return label + ": " + value + "<br>";
    };

    QString htmlBlock =
        "<div style='margin-left:12px; font-family:monospace; white-space:pre-wrap;'>";

    htmlBlock += addLine(tr("Git commit"), FLUVEL_GIT_COMMIT);
    htmlBlock += addLine(tr("Build type"), buildType);
    htmlBlock += addLine(tr("Build origin"), FLUVEL_BUILD_ORIGIN);
    htmlBlock += addLine(tr("Build date (UTC)"), formattedBuildDate());
    htmlBlock += addLine(tr("Qt (build)"), QString(QT_VERSION_STR));
    htmlBlock += addLine(tr("Compiler"), compilerInfo());
    htmlBlock += addLine(tr("CMake"), FLUVEL_CMAKE_VERSION);

    htmlBlock += "</div>";

    html += htmlBlock;

    // =========================
    // Fingerprint
    // =========================

    html += "<div style='font-size:12pt; font-weight:bold; margin-top:14px;'>" +
            tr("Build fingerprint") + "</div>";

    html +=
        "<div style='margin-left:12px; font-family:monospace;'>" + buildFingerprint() + "</div>";

    return html;
}

void AboutWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;

    settings.setValue("ui_geometry/about_window", saveGeometry());
    settings.setValue("ui_geometry/license_window", licenseWindow_->saveGeometry());

    QDialog::closeEvent(event);
}

} // namespace fluvel_app
