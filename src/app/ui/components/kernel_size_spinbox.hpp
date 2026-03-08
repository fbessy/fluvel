// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QSpinBox>

namespace fluvel_app
{

class KernelSizeSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit KernelSizeSpinBox(QWidget* parent = nullptr);

private slots:
    void set_suffix(const QString& text);

protected:
    QValidator::State validate(QString& text, int& pos) const override;
    void focusInEvent(QFocusEvent* event) override;

private:
    QValidator::State previous_result_{QValidator::Acceptable};
};

} // namespace fluvel_app
