// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "application_settings.hpp"
#include "frame_clock.hpp"
#include "image_window.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QTranslator>

#ifdef Q_OS_ANDROID
#include <QQmlApplicationEngine>
#include <QQmlContext>
#endif

int main(int argc, char* argv[])
{
#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);
#endif

    QCoreApplication::setOrganizationName("fluvel");
    QCoreApplication::setOrganizationDomain("https://sourceforge.net/projects/fastlevelset/");
    QCoreApplication::setApplicationName("fluvel");

    fluvel_app::FrameClock::init();

    const auto& config = fluvel_app::ApplicationSettings::instance();
    fluvel_app::Language language = config.appLanguage();

    static QTranslator translator_qt;
    static QTranslator translator_fluvel;

    QString locale;
    switch (language)
    {
        case fluvel_app::Language::System:
            locale = QLocale::system().name().section('_', 0, 0);
            break;
        case fluvel_app::Language::French:
            locale = "fr";
            break;
        case fluvel_app::Language::English:
        default:
            locale = "en";
            break;
    }

    // Traductions Qt
    if (translator_qt.load("qt_" + locale, QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
    {
        app.installTranslator(&translator_qt);
    }

    // Traductions Fluvel
    if (translator_fluvel.load(QString(":/i18n/fluvel_%1.qm").arg(locale)))
    {
        app.installTranslator(&translator_fluvel);
    }

    QIcon appIcon;
    appIcon.addFile(":/icons/fluvel_16.png", QSize(16, 16));
    appIcon.addFile(":/icons/fluvel_22.png", QSize(22, 22));
    appIcon.addFile(":/icons/fluvel_32.png", QSize(32, 32));
    appIcon.addFile(":/icons/fluvel_48.png", QSize(48, 48));
    appIcon.addFile(":/icons/fluvel_64.png", QSize(64, 64));
    appIcon.addFile(":/icons/fluvel_128.png", QSize(128, 128));
    appIcon.addFile(":/icons/fluvel_256.png", QSize(256, 256));

    QApplication::setWindowIcon(appIcon);

    std::unique_ptr<QMainWindow> root;

#ifdef Q_OS_ANDROID
    // root = std::make_unique<fluvel_app::CameraWindow>();
    // root->show();
    QQmlApplicationEngine engine;
    // engine.loadFromModule("fluvel", "Main");

    // #include <QDir>
    // #include <QDebug>

    // qDebug() << "Resources root:" << QDir(":/").entryList(QDir::Files | QDir::Dirs);
    // qDebug() << "Resources qml:" << QDir(":/qml").entryList(QDir::Files | QDir::Dirs);

    engine.load(QUrl("qrc:/qml/Main.qml"));

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []()
        {
            qDebug() << "QML CREATION FAILED";
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    if (engine.rootObjects().isEmpty())
    {
        return -1;
    }
#else
    root = std::make_unique<fluvel_app::ImageWindow>();
    root->show();
#endif

    return app.exec();
}
