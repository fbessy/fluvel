// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QTabWidget>

namespace fluvel
{

/**
 * @brief Tab widget providing animated page transitions.
 *
 * AnimatedTabWidget extends QTabWidget by applying a short slide
 * and fade animation whenever the current page changes.
 *
 * The animation is intentionally subtle to improve perceived
 * responsiveness while preserving the native Qt widget appearance.
 */
class AnimatedTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs an animated tab widget.
     *
     * @param parent Parent widget.
     */
    explicit AnimatedTabWidget(QWidget* parent = nullptr);

private slots:
    /**
     * @brief Animates the newly selected page.
     *
     * A short slide and fade animation is applied to the page
     * corresponding to @p index.
     *
     * @param index Index of the current page.
     */
    void animateCurrentPage(int index);
};

} // namespace fluvel