// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "kernel_size_spinbox.hpp"

namespace ofeli_app
{

KernelSizeSpinBox::KernelSizeSpinBox(QWidget* parent)
    : QSpinBox(parent)
{
    setSuffix(" × ");

    connect(this, QOverload<const QString&>::of(&QSpinBox::textChanged), this,
            &KernelSizeSpinBox::set_suffix, Qt::UniqueConnection);
}

QValidator::State KernelSizeSpinBox::validate(QString& text, int& pos) const
{
    QString text_without_suffix(text);
    text_without_suffix.chop(suffix().size());

    qsizetype size = text_without_suffix.size();
    bool is_successed;
    int value = text_without_suffix.toInt(&is_successed);

    QValidator::State result;

    if (text_without_suffix.isEmpty())
    {
        result = QValidator::Intermediate;
    }
    else if (!is_successed)
    {
        result = QValidator::Invalid;
    }
    else if (value > maximum())
    {
        result = QValidator::Invalid;
    }
    else if (value < minimum())
    {
        result = QValidator::Intermediate;
    }
    else if (value % 2 == 1)
    {
        result = QValidator::Acceptable;
    }
    else if (pos == size)
    {
        result = QValidator::Intermediate;
    }
    else
    {
        result = QValidator::Invalid;
    }

    if (hasFocus())
    {
        if (result == QValidator::Acceptable)
        {
            const_cast<KernelSizeSpinBox*>(this)->setStyleSheet("color:green");
        }
        else if (result == QValidator::Intermediate)
        {
            const_cast<KernelSizeSpinBox*>(this)->setStyleSheet("color:orange");
        }
        else if (result == QValidator::Invalid)
        {
            if (previous_result_ == QValidator::Acceptable)
            {
                const_cast<KernelSizeSpinBox*>(this)->setStyleSheet("color:green");
            }
            else
            {
                const_cast<KernelSizeSpinBox*>(this)->setStyleSheet("color:orange");
            }
        }
    }
    else
    {
        const_cast<KernelSizeSpinBox*>(this)->setStyleSheet("");
    }

    if (result != QValidator::Invalid)
    {
        const_cast<KernelSizeSpinBox*>(this)->previous_result_ = result;
    }

    return result;
}

void KernelSizeSpinBox::set_suffix(const QString& text)
{
    QString text_without_suffix(text);
    text_without_suffix.chop(suffix().size());

    text_without_suffix = " × " + text_without_suffix;
    const_cast<KernelSizeSpinBox*>(this)->setSuffix(text_without_suffix);
}

void KernelSizeSpinBox::focusInEvent(QFocusEvent* event)
{
    setStyleSheet("color:green");
    QAbstractSpinBox::focusInEvent(event);
}

} // namespace ofeli_app
