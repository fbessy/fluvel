// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "application_settings_types.hpp"

#include <QTranslator>

class QCoreApplication;

namespace fluvel
{

/**
 * @brief Installs the Qt and application translations for a language.
 *
 * The translators are installed by the constructor and removed automatically
 * when the object is destroyed. Create it once in main(), right after the
 * QCoreApplication, and keep it alive for the whole run.
 *
 * A language change only takes effect at the next start.
 */
class Translator final
{
public:
    /**
     * @param app      Application receiving the translators.
     * @param language Language selected by the user (System follows the
     *                 system UI languages).
     */
    Translator(QCoreApplication& app, Language language);

    Translator(const Translator&) = delete;
    Translator& operator=(const Translator&) = delete;

private:
    QTranslator qtTranslator_;  //!< Qt own strings (standard dialog buttons, ...).
    QTranslator appTranslator_; //!< Fluvel strings, embedded as :/i18n/fluvel_<lang>.qm.
};

} // namespace fluvel
