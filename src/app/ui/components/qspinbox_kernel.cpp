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

#include "qspinbox_kernel.hpp"

namespace ofeli_gui
{

QSpinBoxKernel::QSpinBoxKernel(QWidget* parent) :
    QSpinBox(parent), previous_result(QValidator::Acceptable)
{
    setSuffix(" × ");
    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(set_suffix(const QString&)), Qt::UniqueConnection );
}

QValidator::State QSpinBoxKernel::validate(QString& text, int& pos) const
{
    QString text_without_suffix(text);
    text_without_suffix.chop( suffix().size() );

    int size = text_without_suffix.size();
    bool is_successed;
    int value = text_without_suffix.toInt(&is_successed);

    QValidator::State result;

    if( text_without_suffix.isEmpty() )
    {
        result = QValidator::Intermediate;
    }
    else if( !is_successed )
    {
        result = QValidator::Invalid;
    }
    else if( value > maximum() )
    {
        result = QValidator::Invalid;
    }
    else if( value < minimum() )
    {
        result = QValidator::Intermediate;
    }
    else if( value % 2 == 1 )
    {
        result = QValidator::Acceptable;
    }
    else if( pos == size )
    {
        result = QValidator::Intermediate;
    }
    else
    {
        result = QValidator::Invalid;
    }

    if( hasFocus() )
    {

        if( result == QValidator::Acceptable )
        {
            const_cast<QSpinBoxKernel*>(this)->setStyleSheet("color:green");
        }
        else if( result == QValidator::Intermediate )
        {
            const_cast<QSpinBoxKernel*>(this)->setStyleSheet("color:orange");
        }
        else if( result == QValidator::Invalid )
        {
            if( previous_result == QValidator::Acceptable )
            {
                const_cast<QSpinBoxKernel*>(this)->setStyleSheet("color:green");
            }
            else
            {
                const_cast<QSpinBoxKernel*>(this)->setStyleSheet("color:orange");
            }
        }
    }
    else
    {
        const_cast<QSpinBoxKernel*>(this)->setStyleSheet("");
    }

    if( result != QValidator::Invalid )
    {
        const_cast<QSpinBoxKernel*>(this)->previous_result = result;
    }

    return result;
}

void QSpinBoxKernel::set_suffix(const QString& text)
{
    QString text_without_suffix(text);
    text_without_suffix.chop( suffix().size() );

    text_without_suffix = " × " + text_without_suffix;
    const_cast<QSpinBoxKernel*>(this)->setSuffix(text_without_suffix);
}

void QSpinBoxKernel::focusInEvent(QFocusEvent* event)
{
    setStyleSheet("color:green");
    QAbstractSpinBox::focusInEvent(event);
}

}
