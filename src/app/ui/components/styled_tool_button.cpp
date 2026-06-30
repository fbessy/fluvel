// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "styled_tool_button.hpp"
#include "qcolor_utils.hpp"
#include "ui_theme.hpp"

#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QtMath>

namespace
{

constexpr int kFlipDurationMs = 220;
constexpr qreal kFlipAngle = 80.0;

} // namespace

namespace fluvel
{

StyledToolButton::StyledToolButton(QWidget* parent, ui::Appearance appearance)
    : QToolButton(parent)
    , appearance_(appearance)
{
    updateStyle();

    setCursor(Qt::PointingHandCursor);

    const auto bSize = ui::kButtonSize;
    const auto iSize = ui::kButtonIconSize;

    setFixedSize(bSize, bSize);
    setIconSize(QSize(iSize, iSize));
}

ui::Appearance StyledToolButton::appearance() const
{
    return appearance_;
}

void StyledToolButton::setAppearance(ui::Appearance appearance)
{
    if (appearance_ == appearance)
        return;

    appearance_ = appearance;

    updateStyle();
}

void StyledToolButton::updateStyle()
{
    if (appearance_ == ui::Appearance::Native)
    {
        setStyleSheet({});
        return;
    }

    setStyleSheet(QString(R"(

QToolButton
{
    background: %1;
    border: 1px solid %2;
    border-radius: %3px;
}

QToolButton:hover
{
    background: %4;
}

QToolButton:pressed
{
    background: %5;
}

QToolButton:disabled
{
    background: %6;
    border: 1px solid %7;
}

QToolButton:checked
{
    background: %8;
    border: 1px solid %8;
}

QToolButton:checked:hover
{
    background: %9;
    border: 1px solid %9;
}

)")
                      .arg(qcolor_utils::rgba(ui::kControlBackground))
                      .arg(qcolor_utils::rgba(ui::kControlBorder))
                      .arg(ui::kControlRadius)
                      .arg(qcolor_utils::rgba(ui::kControlHover))
                      .arg(qcolor_utils::rgba(ui::kControlPressed))
                      .arg(qcolor_utils::rgba(ui::kControlDisabled))
                      .arg(qcolor_utils::rgba(ui::kControlDisabledBorder))
                      .arg(qcolor_utils::rgba(ui::kAccentColor))
                      .arg(qcolor_utils::rgba(ui::kAccentHoverColor)));
}

void StyledToolButton::paintEvent(QPaintEvent*)
{
    QStylePainter painter(this);

    QStyleOptionToolButton option;
    initStyleOption(&option);

    // Draw the button using the current Qt style but
    // suppress the icon since it will be rendered manually.
    option.icon = {};

    painter.drawComplexControl(QStyle::CC_ToolButton, option);

    const QPointF center(width() / 2.0, height() / 2.0);

    // Draw one icon of the animated transition.
    // The 3D flip illusion is produced by combining
    // a slight rotation with a horizontal compression.
    auto drawIcon = [&](const QIcon& icon, qreal opacity, qreal angle)
    {
        if (icon.isNull() || opacity <= 0.0)
            return;

        const QPixmap pixmap = icon.pixmap(iconSize());

        const qreal sx = std::abs(std::cos(qDegreesToRadians(angle)));

        painter.save();

        painter.translate(center);

        painter.rotate(angle);

        painter.scale(sx, 1.0);

        painter.setOpacity(opacity);

        painter.translate(-pixmap.width() / 2.0, -pixmap.height() / 2.0);

        painter.drawPixmap(QPointF(), pixmap);

        painter.restore();
    };

    if (currentIcon_.isNull())
    {
        drawIcon(icon(), 1.0, 0.0);
        return;
    }

    const qreal p = transitionProgress_;

    const qreal angle = transitionDirection_ * kFlipAngle * std::sin(p * M_PI);

    drawIcon(currentIcon_, 1.0 - p, angle);

    drawIcon(nextIcon_, p, -angle);
}

void StyledToolButton::setAnimatedIcon(const QIcon& icon, FlipDirection direction)
{
    const QIcon current = this->icon();

    if (icon.cacheKey() == current.cacheKey())
        return;

    currentIcon_ = current;
    nextIcon_ = icon;

    updateTransitionDirection(direction);

    if (transitionAnimation_)
    {
        transitionAnimation_->stop();
        transitionProgress_ = 0.0;
    }

    transitionAnimation_ = new QPropertyAnimation(this, "transitionProgress");

    transitionAnimation_->setDuration(kFlipDurationMs);
    transitionAnimation_->setStartValue(0.0);
    transitionAnimation_->setEndValue(1.0);
    transitionAnimation_->setEasingCurve(QEasingCurve::InOutSine);

    connect(transitionAnimation_, &QPropertyAnimation::finished, this,
            [this]()
            {
                setIcon(nextIcon_);

                currentIcon_ = {};
                nextIcon_ = {};

                transitionProgress_ = 0.0;

                transitionAnimation_ = nullptr;

                update();
            });

    transitionAnimation_->start(QAbstractAnimation::DeleteWhenStopped);
}

qreal StyledToolButton::transitionProgress() const
{
    return transitionProgress_;
}

void StyledToolButton::setTransitionProgress(qreal progress)
{
    if (qFuzzyCompare(progress, transitionProgress_))
        return;

    transitionProgress_ = progress;
    update();
}

void StyledToolButton::updateTransitionDirection(FlipDirection direction)
{
    switch (direction)
    {
        case FlipDirection::Left:
            transitionDirection_ = -1.0;
            break;

        case FlipDirection::Right:
            transitionDirection_ = 1.0;
            break;

        case FlipDirection::Auto:
            transitionDirection_ *= -1.0;
            break;
    }
}

} // namespace fluvel