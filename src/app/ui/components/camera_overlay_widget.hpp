// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "camera_stats.hpp"

#include <QWidget>
#include <QLabel>

namespace ofeli_app
{

class CameraOverlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraOverlayWidget(QWidget* parent = nullptr);

public slots:
    void setStats(const ofeli_app::CameraStats& stats);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QLabel* label_;
};

} // namespace ofeli_app
