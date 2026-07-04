// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QObject>
#include <QPointer>

class QWidget;
class QPropertyAnimation;

namespace fluvel
{

/**
 * @brief Animated scale feedback for clickable widgets.
 *
 * ScaleAnimation provides a short press animation by temporarily
 * shrinking the target widget before restoring its original size.
 *
 * The animation is independent from icon transitions and can be
 * reused by any custom widget implementing its own paintEvent().
 */
class ScaleAnimation : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal scale READ scale WRITE setScale)

public:
    /**
     * @brief Constructs a scale animation.
     *
     * @param owner Widget updated during the animation.
     */
    explicit ScaleAnimation(QWidget* owner);

    /**
     * @brief Starts the click animation.
     *
     * If a previous animation is still running, it is restarted
     * from the beginning.
     */
    void start();

    /**
     * @brief Returns the current scale factor.
     */
    qreal scale() const;

private:
    /**
     * @brief Updates the current scale factor.
     */
    void setScale(qreal scale);

private:
    /// Target widget.
    QPointer<QWidget> owner_;

    /// Current running animation.
    QPointer<QPropertyAnimation> animation_;

    /// Current scale factor.
    qreal scale_{1.0};
};

} // namespace fluvel