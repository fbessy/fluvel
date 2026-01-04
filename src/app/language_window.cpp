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

#include <QtWidgets>

namespace ofeli_gui
{

LanguageWindow::LanguageWindow(QWidget* parent) :
    QDialog(parent),
    language(AppSettings::instance().app_language)
{
    setWindowTitle( tr("Language") );

    QSettings settings;
    const auto geo = settings.value("Language/Window/geometry").toByteArray();
    if (!geo.isEmpty())
        restoreGeometry(geo);

    list_widget = new QListWidget(this);

    QString locale = QLocale::system().name().section('_', 0, 0);

    list_widget->addItem( tr("System (")+locale+")" );
    list_widget->addItem( tr("English") );
    list_widget->addItem( tr("French") );
    list_widget->setCurrentRow( int(language) );

    QDialogButtonBox* buttons = new QDialogButtonBox(this);
    buttons->addButton(QDialogButtonBox::Ok);
    buttons->addButton(QDialogButtonBox::Cancel);
    buttons->setCenterButtons(true);
    connect( buttons, SIGNAL(accepted()), this, SLOT(accept()) );
    connect( buttons, SIGNAL(rejected()), this, SLOT(reject()) );

    QLabel* restart_label = new QLabel(this);
    restart_label->setAlignment(Qt::AlignJustify);
    restart_label->setWordWrap(true);
    restart_label->setText(tr("The change will take effect after restarting the application."));

    QVBoxLayout* layout_this = new QVBoxLayout;
    layout_this->addWidget(list_widget);
    layout_this->addWidget(restart_label);
    layout_this->addWidget(buttons);
    layout_this->setSizeConstraint(QLayout::SetMinimumSize);

    setLayout(layout_this);
}

void LanguageWindow::apply_setting()
{
    language = Language( list_widget->currentRow() );
}

void LanguageWindow::cancel_setting()
{
    list_widget->setCurrentRow( language );
}

void LanguageWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;
    settings.setValue("Language/Window/geometry", saveGeometry());

    QDialog::closeEvent(event);
}


}
