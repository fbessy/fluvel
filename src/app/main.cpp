// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#ifdef FLUVEL_PLATFORM_WINDOWS
#include <shobjidl_core.h>
#include <windows.h>
#endif

// clang-format off
#if defined(FLUVEL_PLATFORM_MOBILE)
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
#include "icon_loader.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QLibraryInfo>
#include <QTranslator>

int main(int argc, char* argv[])
{
#if defined(FLUVEL_PLATFORM_MOBILE) && !defined(FLUVEL_UI_DESKTOP)
    QGuiApplication app(argc, argv);
#else
    QApplication app(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);
#endif

#ifdef FLUVEL_PLATFORM_WINDOWS
    SetCurrentProcessExplicitAppUserModelID(L"org.fluvel.Fluvel");
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

#ifdef FLUVEL_PLATFORM_DESKTOP
    QApplication::setWindowIcon(fluvel_app::il::desktopAppIcon());
#endif

#if defined(FLUVEL_PLATFORM_MOBILE) && !defined(FLUVEL_UI_DESKTOP)

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

#if defined(FLUVEL_PLATFORM_MOBILE)
    auto root = std::make_unique<fluvel_app::CameraWindow>();
#else
    auto root = std::make_unique<fluvel_app::ImageWindow>();
#endif

    root->show();

#endif

    return app.exec();
}
