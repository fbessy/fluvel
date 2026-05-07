// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include <QtGlobal>

#ifdef Q_OS_WIN
#include <shobjidl_core.h>
#include <windows.h>
#endif

// clang-format off
#if defined(Q_OS_ANDROID)
    #if defined(FLUVEL_UI_DESKTOP)
        #include "camera_window.hpp"
    #else
        #include <QQmlApplicationEngine>
        #include <QQmlContext>
    #endif
#else
    #include "image_window.hpp"
#endif
// clang-format on

#include "application_settings.hpp"
#include "frame_clock.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QLibraryInfo>
#include <QStyle>
#include <QTranslator>

int main(int argc, char* argv[])
{
#if defined(Q_OS_ANDROID) && !defined(FLUVEL_UI_DESKTOP)
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);
#endif

#ifdef Q_OS_LINUX
    if (qEnvironmentVariableIsSet("APPIMAGE"))
    {
        const QString style = QApplication::style()->objectName().toLower();

        if (style.isEmpty() || style == "windows")
            QApplication::setStyle("Fusion");
    }
#endif

#ifdef Q_OS_WIN
    SetCurrentProcessExplicitAppUserModelID(L"org.fluvel.app");
#endif

    QCoreApplication::setOrganizationName("fluvel");
    QCoreApplication::setOrganizationDomain("fluvel.org");
    QCoreApplication::setApplicationName("fluvel");

    QGuiApplication::setApplicationDisplayName("Fluvel");

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

    if (translator_qt.load("qt_" + locale, QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&translator_qt);

    if (translator_fluvel.load(QString(":/i18n/fluvel_%1.qm").arg(locale)))
        app.installTranslator(&translator_fluvel);

    // Icône (desktop uniquement utile, mais safe partout)
    QIcon appIcon;
    appIcon.addFile(":/icons/fluvel_16.png", QSize(16, 16));
    appIcon.addFile(":/icons/fluvel_22.png", QSize(22, 22));
    appIcon.addFile(":/icons/fluvel_32.png", QSize(32, 32));
    appIcon.addFile(":/icons/fluvel_48.png", QSize(48, 48));
    appIcon.addFile(":/icons/fluvel_64.png", QSize(64, 64));
    appIcon.addFile(":/icons/fluvel_128.png", QSize(128, 128));
    appIcon.addFile(":/icons/fluvel_256.png", QSize(256, 256));

#ifndef Q_OS_ANDROID
    QApplication::setWindowIcon(appIcon);
#endif

#if defined(Q_OS_ANDROID) && !defined(FLUVEL_UI_DESKTOP)

    QQmlApplicationEngine engine;
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
        return -1;

#else

#if defined(Q_OS_ANDROID)
    auto root = std::make_unique<fluvel_app::CameraWindow>();
#else
    auto root = std::make_unique<fluvel_app::ImageWindow>();
#endif

    root->show();

#endif

    return app.exec();
}
