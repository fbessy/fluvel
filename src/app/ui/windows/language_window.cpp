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

#include "language_window.hpp"
#include "application_settings.hpp"

#include <QComboBox>
#include <QSettings>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

namespace ofeli_app
{

LanguageWindow::LanguageWindow(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Language"));

    QSettings settings;
    const auto geo = settings.value("Language/Window/geometry").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    // --- Combo box ---
    combo = new QComboBox(this);

    QString locale = QLocale::system().name().section('_', 0, 0);

    combo->addItem(tr("System (%1)").arg(locale),
                   QVariant::fromValue(int(Language::System)));

    combo->addItem(tr("English"),
                   QVariant::fromValue(int(Language::English)));

    combo->addItem(tr("French"),
                   QVariant::fromValue(int(Language::French)));

    // sélectionner la langue actuelle indépendamment de l’index
    auto currentLanguage = AppSettings::instance().app_language;

    int index = combo->findData(QVariant::fromValue(int(currentLanguage)));
    if (index >= 0)
        combo->setCurrentIndex(index);

    // --- Buttons ---
    QDialogButtonBox* buttons = new QDialogButtonBox(this);
    buttons->addButton(QDialogButtonBox::Ok);
    buttons->addButton(QDialogButtonBox::Cancel);
    buttons->setCenterButtons(true);

    connect(buttons, &QDialogButtonBox::accepted,
            this,    &LanguageWindow::accept);

    connect(buttons, &QDialogButtonBox::rejected,
            this,    &LanguageWindow::reject);

    // --- Restart label ---
    QLabel* restart_label = new QLabel(this);
    restart_label->setAlignment(Qt::AlignJustify);
    restart_label->setWordWrap(true);
    restart_label->setText(tr("The change will take effect after restarting the application."));

    // --- Layout ---
    QVBoxLayout* layout_this = new QVBoxLayout;
    layout_this->addWidget(combo);
    layout_this->addWidget(restart_label);
    layout_this->addWidget(buttons);
    layout_this->setSizeConstraint(QLayout::SetMinimumSize);

    setLayout(layout_this);
}

void LanguageWindow::accept()
{
    // récupérer la valeur logique (pas l’index)
    Language language = Language(
        combo->currentData().toInt()
    );

    AppSettings::instance().app_language = language;

    QSettings settings;
    settings.setValue("Language/current", int(language));

    QDialog::accept();
}

void LanguageWindow::reject()
{
    // restaurer la langue active
    auto language = AppSettings::instance().app_language;

    int index = combo->findData(QVariant::fromValue(int(language)));
    if (index >= 0)
        combo->setCurrentIndex(index);

    QDialog::reject();
}

void LanguageWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;
    settings.setValue("Language/Window/geometry", saveGeometry());

    QDialog::closeEvent(event);
}

}
