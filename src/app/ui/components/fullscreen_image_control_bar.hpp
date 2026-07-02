// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QWidget>

class QPushButton;
class QHBoxLayout;

namespace fluvel
{

class AnimatedPushButton;

/**
 * @brief Fullscreen control overlay for image sessions.
 *
 * This widget provides a compact control bar displayed on top of the
 * image viewer when the application is in fullscreen mode.
 *
 * It exposes the same controls as the standard image session toolbar:
 * - start / restart
 * - pause / resume
 * - step
 * - converge
 *
 * The widget is intentionally lightweight and contains no application
 * logic. State updates, icons and button behavior are managed by
 * ImageWindow.
 */
class FullscreenImageControlBar : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the fullscreen control bar.
     *
     * @param parent Parent widget.
     */
    explicit FullscreenImageControlBar(QWidget* parent = nullptr);

    /**
     * @brief Returns the start/restart button.
     */
    AnimatedPushButton* restartButton() const;

    /**
     * @brief Returns the pause/resume button.
     */
    AnimatedPushButton* pauseButton() const;

    /**
     * @brief Returns the single-step execution button.
     */
    QPushButton* stepButton() const;

    /**
     * @brief Returns the convergence button.
     */
    QPushButton* convergeButton() const;

private:
    AnimatedPushButton* restartButton_{nullptr};
    AnimatedPushButton* pauseButton_{nullptr};
    QPushButton* stepButton_{nullptr};
    QPushButton* convergeButton_{nullptr};
};

} // namespace fluvel