// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "language_window.hpp"
#include "application_settings.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSettings>
#include <QVBoxLayout>

namespace fluvel_app
{

LanguageWindow::LanguageWindow(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Language"));

    QSettings settings;
    const auto geo = settings.value("ui_geometry/language_window").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    // --- Combo box ---
    combo_ = new QComboBox(this);

    QString locale = QLocale::system().name().section('_', 0, 0);

    combo_->addItem(tr("System (%1)").arg(locale), QVariant::fromValue(int(Language::System)));

    combo_->addItem(tr("English"), QVariant::fromValue(int(Language::English)));

    combo_->addItem(tr("French"), QVariant::fromValue(int(Language::French)));

    // sélectionner la langue actuelle indépendamment de l’index
    auto currentLanguage = AppSettings::instance().app_language;

    int index = combo_->findData(QVariant::fromValue(int(currentLanguage)));
    if (index >= 0)
        combo_->setCurrentIndex(index);

    // --- Buttons ---
    QDialogButtonBox* buttons = new QDialogButtonBox(this);
    buttons->addButton(QDialogButtonBox::Ok);
    buttons->addButton(QDialogButtonBox::Cancel);
    buttons->setCenterButtons(true);

    connect(buttons, &QDialogButtonBox::accepted, this, &LanguageWindow::accept);

    connect(buttons, &QDialogButtonBox::rejected, this, &LanguageWindow::reject);

    // --- Restart label ---
    QLabel* restart_label = new QLabel(this);
    restart_label->setAlignment(Qt::AlignJustify);
    restart_label->setWordWrap(true);
    restart_label->setText(tr("The change will take effect after restarting the application."));

    // --- Layout ---
    QVBoxLayout* layout_this = new QVBoxLayout;
    layout_this->addWidget(combo_);
    layout_this->addWidget(restart_label);
    layout_this->addWidget(buttons);
    layout_this->setSizeConstraint(QLayout::SetMinimumSize);

    setLayout(layout_this);
}

void LanguageWindow::accept()
{
    // récupérer la valeur logique (pas l’index)
    Language language = Language(combo_->currentData().toInt());

    AppSettings::instance().app_language = language;

    QSettings settings;
    settings.setValue("ui/language", int(language));

    QDialog::accept();
}

void LanguageWindow::reject()
{
    // restaurer la langue active
    auto language = AppSettings::instance().app_language;

    int index = combo_->findData(QVariant::fromValue(int(language)));
    if (index >= 0)
        combo_->setCurrentIndex(index);

    QDialog::reject();
}

void LanguageWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;
    settings.setValue("ui_geometry/language_window", saveGeometry());

    QDialog::closeEvent(event);
}

} // namespace fluvel_app
