/****************************************************************************
**
** Copyright (C) 2010-2025 Fabien Bessy.
** Contact: fabien.bessy@gmail.com
**
** This file is part of project Ofeli.
**
** http://www.cecill.info/licences/Licence_CeCILL_V2-en.html
** You may use this file under the terms of the CeCILL license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Fabien Bessy and its Subsidiary(-ies) nor the
**     names of its contributors may be used to endorse or promote products
**     derived from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

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

    QCoreApplication::setOrganizationName("ofeli");
    QCoreApplication::setOrganizationDomain("https://sourceforge.net/projects/fastlevelset/");
    QCoreApplication::setApplicationName("ofeli");

    ofeli_app::FrameClock::init();

    const auto& config = ofeli_app::AppSettings::instance();
    ofeli_app::Language language = config.app_language;

    static QTranslator translator_qt;
    static QTranslator translator_ofeli;

    QString locale;
    switch (language)
    {
        case ofeli_app::Language::System:
            locale = QLocale::system().name().section('_', 0, 0);
            break;
        case ofeli_app::Language::French:
            locale = "fr";
            break;
        case ofeli_app::Language::English:
        default:
            locale = "en";
            break;
    }

    // Traductions Qt
    if (translator_qt.load("qt_" + locale, QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
    {
        app.installTranslator(&translator_qt);
    }

    // Traductions Ofeli
    if (translator_ofeli.load(QString(":/i18n/Ofeli_%1.qm").arg(locale)))
    {
        app.installTranslator(&translator_ofeli);
    }

    QIcon appIcon;
    appIcon.addFile(":/icons/Ofeli_16.png", QSize(16, 16));
    appIcon.addFile(":/icons/Ofeli_32.png", QSize(32, 32));
    appIcon.addFile(":/icons/Ofeli_48.png", QSize(48, 48));
    appIcon.addFile(":/icons/Ofeli_128.png", QSize(128, 128));
    appIcon.addFile(":/icons/Ofeli_256.png", QSize(256, 256));

    QApplication::setWindowIcon(appIcon);

    std::unique_ptr<QMainWindow> root;

#ifdef Q_OS_ANDROID
    // root = std::make_unique<ofeli_app::CameraWindow>();
    // root->show();
    QQmlApplicationEngine engine;
    // engine.loadFromModule("ofeli", "Main");

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
    root = std::make_unique<ofeli_app::ImageWindow>();
    root->show();
#endif

    return app.exec();
}
