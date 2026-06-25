// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "clickable_label.hpp"

namespace fluvel
{

ClickableLabel::ClickableLabel(QWidget* parent)
    : QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

ClickableLabel::ClickableLabel(const QString& text, QWidget* parent)
    : QLabel(text, parent)
{
    setCursor(Qt::PointingHandCursor);
}

} // namespace fluvel