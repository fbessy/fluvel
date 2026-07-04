// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QTabWidget>

namespace fluvel
{

class AnimatedTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit AnimatedTabWidget(QWidget* parent = nullptr)
        : QTabWidget(parent)
    {
        connect(this, &QTabWidget::currentChanged, this, &AnimatedTabWidget::animateCurrentPage);
    }

private slots:
    void animateCurrentPage(int index)
    {
        QWidget* page = widget(index);

        if (!page)
            return;

        const QPoint finalPos = page->pos();

        //
        // Slide animation
        //
        page->move(finalPos.x() + 10, finalPos.y());

        auto* slideAnim = new QPropertyAnimation(page, "pos");
        slideAnim->setDuration(100);
        slideAnim->setStartValue(page->pos());
        slideAnim->setEndValue(finalPos);
        slideAnim->setEasingCurve(QEasingCurve::OutCubic);

        //
        // Fade animation
        //
        auto* effect = new QGraphicsOpacityEffect(page);
        page->setGraphicsEffect(effect);

        effect->setOpacity(0.90);

        auto* fadeAnim = new QPropertyAnimation(effect, "opacity");
        fadeAnim->setDuration(100);
        fadeAnim->setStartValue(0.90);
        fadeAnim->setEndValue(1.0);
        fadeAnim->setEasingCurve(QEasingCurve::OutCubic);

        //
        // Run both animations together
        //
        auto* group = new QParallelAnimationGroup(this);

        group->addAnimation(slideAnim);
        group->addAnimation(fadeAnim);

        connect(group, &QParallelAnimationGroup::finished, effect, &QObject::deleteLater);

        group->start(QAbstractAnimation::DeleteWhenStopped);
    }
};

} // namespace fluvel