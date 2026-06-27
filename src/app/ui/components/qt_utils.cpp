// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "qt_utils.hpp"

#include <QSignalBlocker>

namespace fluvel::qt_utils
{

void copyComboBox(const QComboBox* source, QComboBox* destination)
{
    if (!source || !destination)
        return;

    QSignalBlocker blocker(destination);

    destination->clear();

    for (int i = 0; i < source->count(); ++i)
    {
        destination->addItem(source->itemIcon(i), source->itemText(i), source->itemData(i));

        destination->setItemData(i, source->itemData(i, Qt::ToolTipRole), Qt::ToolTipRole);

        destination->setItemData(i, source->itemData(i, Qt::StatusTipRole), Qt::StatusTipRole);

        destination->setItemData(i, source->itemData(i, Qt::WhatsThisRole), Qt::WhatsThisRole);

        destination->setItemData(i, source->itemData(i, Qt::UserRole), Qt::UserRole);
    }

    destination->setCurrentIndex(source->currentIndex());

    destination->setEnabled(source->isEnabled());

    destination->setToolTip(source->toolTip());
}

} // namespace fluvel::qt_utils