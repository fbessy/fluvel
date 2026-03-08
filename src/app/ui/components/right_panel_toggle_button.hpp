// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QIcon>
#include <QPushButton>
#include <QWidget>

namespace fluvel_app
{

class RightPanelToggleButton : public QPushButton
{
    Q_OBJECT

public:
    explicit RightPanelToggleButton(QWidget* parent = nullptr);

private slots:
    void updateAppearance(bool checked);

private:
    QIcon iconOn_;
    QIcon iconOff_;
};

} // namespace fluvel_app
