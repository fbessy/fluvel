// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "translator.hpp"

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>

namespace fluvel
{

namespace
{

QLocale localeFor(Language language)
{
    switch (language)
    {
        case Language::French:
            return QLocale(QLocale::French);
        case Language::English:
            return QLocale(QLocale::English);
        case Language::System:
        default:
            return QLocale::system();
    }
}

} // namespace

Translator::Translator(QCoreApplication& app, Language language)
{
    const QLocale locale = localeFor(language);

    // QTranslator::load(QLocale, ...) tries the locale UI languages from the most
    // to the least specific (e.g. fluvel_pt_BR.qm, then fluvel_pt.qm).
    if (qtTranslator_.load(locale, QStringLiteral("qt"), QStringLiteral("_"),
                           QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
    {
        app.installTranslator(&qtTranslator_);
    }

    // No file is expected for English, the source language.
    if (appTranslator_.load(locale, QStringLiteral("fluvel"), QStringLiteral("_"),
                            QStringLiteral(":/i18n")))
    {
        app.installTranslator(&appTranslator_);
    }
}

} // namespace fluvel
