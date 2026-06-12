// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "language_dialog.hpp"
#include "application_settings.hpp"

#include <QAbstractButton>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QSettings>
#include <QString>
#include <QVBoxLayout>

namespace fluvel
{

LanguageDialog::LanguageDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Language"));

    QSettings settings;
    const auto geo = settings.value("ui_geometry/language_dialog").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    // --- Combo box ---
    combo_ = new QComboBox(this);

    QString locale = QLocale::system().name().section('_', 0, 0);

    combo_->addItem(tr("System (%1)").arg(locale), QVariant::fromValue(int(Language::System)));

    combo_->addItem(tr("English"), QVariant::fromValue(int(Language::English)));

    combo_->addItem(tr("French"), QVariant::fromValue(int(Language::French)));

    // sélectionner la langue actuelle indépendamment de l’index
    auto currentLanguage = ApplicationSettings::instance().appLanguage();

    int index = combo_->findData(QVariant::fromValue(int(currentLanguage)));
    if (index >= 0)
        combo_->setCurrentIndex(index);

    // --- Buttons ---
    QDialogButtonBox* dialogButtons_ = new QDialogButtonBox(this);
    dialogButtons_->addButton(QDialogButtonBox::Ok);
    dialogButtons_->addButton(QDialogButtonBox::Cancel);
    dialogButtons_->setCenterButtons(true);

#ifdef Q_OS_LINUX

    // AppImage may use inconsistent platform dialog icons.
    // Use text-only dialog buttons for a cleaner cross-platform UI.

    if (qEnvironmentVariableIsSet("APPIMAGE"))
    {
        const auto buttons = dialogButtons_->buttons();

        for (QAbstractButton* button : buttons)
        {
            button->setIcon(QIcon());
        }
    }

#endif

    connect(dialogButtons_, &QDialogButtonBox::accepted, this, &LanguageDialog::accept);

    connect(dialogButtons_, &QDialogButtonBox::rejected, this, &LanguageDialog::reject);

    // --- Restart label ---
    QLabel* restart_label = new QLabel(this);
    restart_label->setAlignment(Qt::AlignJustify);
    restart_label->setWordWrap(true);
    restart_label->setText(tr("The change will take effect after restarting the application."));

    // --- Layout ---
    QVBoxLayout* layout_this = new QVBoxLayout;
    layout_this->addWidget(combo_);
    layout_this->addWidget(restart_label);
    layout_this->addWidget(dialogButtons_);
    layout_this->setSizeConstraint(QLayout::SetMinimumSize);

    setLayout(layout_this);
}

void LanguageDialog::accept()
{
    // récupérer la valeur logique (pas l’index)
    Language language = Language(combo_->currentData().toInt());

    ApplicationSettings::instance().setAppLanguage(language);

    QSettings settings;
    settings.setValue("ui/language", int(language));

    QDialog::accept();
}

void LanguageDialog::reject()
{
    // restaurer la langue active
    auto language = ApplicationSettings::instance().appLanguage();

    int index = combo_->findData(QVariant::fromValue(int(language)));
    if (index >= 0)
        combo_->setCurrentIndex(index);

    QDialog::reject();
}

void LanguageDialog::closeEvent(QCloseEvent* event)
{
    QSettings settings;
    settings.setValue("ui_geometry/language_dialog", saveGeometry());

    QDialog::closeEvent(event);
}

} // namespace fluvel
