// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QLabel>
#include <QObject>

namespace fluvel
{

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget* parent = nullptr);
    explicit ClickableLabel(const QString& text, QWidget* parent = nullptr);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent*) override
    {
        emit clicked();
    }
};

} // namespace fluvel