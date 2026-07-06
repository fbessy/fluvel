// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "animated_push_button.hpp"

#include <QApplication>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QtNumeric>

namespace fluvel
{

AnimatedPushButton::AnimatedPushButton(QWidget* parent)
    : QPushButton(parent)
    , animatedIcon_(this)
{
}

AnimatedPushButton::AnimatedPushButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
    , animatedIcon_(this)
{
}

AnimatedPushButton::AnimatedPushButton(const QIcon& icon, const QString& text, QWidget* parent)
    : QPushButton(icon, text, parent)
    , animatedIcon_(this)
{
}

void AnimatedPushButton::setAnimatedIcon(const QIcon& icon, TransitionDirection direction)
{
    animatedIcon_.setAnimatedIcon(this->icon(), icon, direction);

    setIcon(icon);
}

void AnimatedPushButton::paintEvent(QPaintEvent*)
{
    QStylePainter painter(this);

    QStyleOptionButton option;
    initStyleOption(&option);

    const ButtonLayout layout = calculateLayout();

    const qreal s = scaleAnimation_.scale();

    if (!qFuzzyCompare(s, 1.0))
    {
        painter.save();

        // Use floating-point coordinates instead of QRect::center() to improve animation precision.
        const QPointF center(layout.iconRect.left() + layout.iconRect.width() * 0.5,
                             layout.iconRect.top() + layout.iconRect.height() * 0.5);

        painter.translate(center);
        painter.scale(s, s);
        painter.translate(-center);
    }

    painter.drawControl(QStyle::CE_PushButtonBevel, option);

    animatedIcon_.paint(painter, layout.iconRect, iconSize(), icon());

    style()->drawItemText(&painter, layout.textRect, Qt::AlignCenter | Qt::AlignVCenter, palette(),
                          isEnabled(), text(), foregroundRole());

    if (!qFuzzyCompare(s, 1.0))
        painter.restore();
}

TransitionEffect AnimatedPushButton::transitionEffect() const
{
    return animatedIcon_.transitionEffect();
}

void AnimatedPushButton::setTransitionEffect(TransitionEffect effect)
{
    animatedIcon_.setTransitionEffect(effect);
}

void AnimatedPushButton::mousePressEvent(QMouseEvent* event)
{
    if (clickAnimation_ == ClickAnimation::Scale)
        scaleAnimation_.start();

    QPushButton::mousePressEvent(event);
}

ClickAnimation AnimatedPushButton::clickAnimation() const
{
    return clickAnimation_;
}

void AnimatedPushButton::setClickAnimation(ClickAnimation animation)
{
    clickAnimation_ = animation;
}

AnimatedPushButton::ButtonLayout AnimatedPushButton::calculateLayout() const
{
    ButtonLayout layout;

    QStyleOptionButton opt;
    initStyleOption(&opt);

    const QRect contents = style()->subElementRect(QStyle::SE_PushButtonContents, &opt, this);

    const bool hasIcon = !icon().isNull();
    const bool hasText = !text().isEmpty();

    const QSize iconSz = hasIcon ? iconSize() : QSize();

    if (hasIcon && !hasText)
    {
        layout.iconRect = QStyle::alignedRect(layoutDirection(), Qt::AlignCenter, iconSz, contents);

        return layout;
    }

    if (!hasIcon && hasText)
    {
        layout.textRect = contents;
        return layout;
    }

    const QFontMetrics fm(font());

    const int textWidth = fm.horizontalAdvance(text());
    const int textHeight = fm.height();

    constexpr int kIconSpacing = 4;

    const int spacing = hasIcon && hasText ? kIconSpacing : 0;

    const int totalWidth = iconSz.width() + spacing + textWidth;
    const int totalHeight = std::max(iconSz.height(), textHeight);

    QRect group(0, 0, totalWidth, totalHeight);
    group.moveCenter(contents.center());

    if (layoutDirection() == Qt::LeftToRight)
    {
        layout.iconRect = QRect(group.left(), group.center().y() - iconSz.height() / 2,
                                iconSz.width(), iconSz.height());

        layout.textRect = QRect(layout.iconRect.right() + 1 + spacing,
                                group.center().y() - textHeight / 2, textWidth, textHeight);
    }
    else
    {
        layout.textRect =
            QRect(group.left(), group.center().y() - textHeight / 2, textWidth, textHeight);

        layout.iconRect =
            QRect(layout.textRect.right() + 1 + spacing, group.center().y() - iconSz.height() / 2,
                  iconSz.width(), iconSz.height());
    }

    return layout;
}

int AnimatedPushButton::recommendedWidth(const QPushButton& reference, const QStringList& labels)
{
    QFontMetrics fm(reference.font());

    int textWidth = 0;

    for (const QString& label : labels)
        textWidth = std::max(textWidth, fm.horizontalAdvance(label));

    const QStyle* style = reference.style();

    const int iconWidth = reference.iconSize().width();
    const int margin = style->pixelMetric(QStyle::PM_ButtonMargin, nullptr, &reference);
    const int spacing = 6;

    return textWidth + iconWidth + spacing + margin * 4;
}

} // namespace fluvel