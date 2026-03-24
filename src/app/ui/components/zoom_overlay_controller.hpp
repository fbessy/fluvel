// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QObject>
#include <QPropertyAnimation>
#include <QTimer>

namespace fluvel_app
{

class OverlayTextItem;

class ZoomOverlayController : public QObject
{
    Q_OBJECT

public:
    explicit ZoomOverlayController(OverlayTextItem* item, QObject* parent = nullptr);

    void show(int percent);

private slots:
    void onTimeout();
    void onFadeFinished();

private:
    void startFade();

private:
    OverlayTextItem* item_ = nullptr;

    QTimer timer_;
    QPropertyAnimation* anim_ = nullptr;

    int displayDurationMs_ = 800;
    int fadeDurationMs_ = 250;
};

} // namespace fluvel_app
