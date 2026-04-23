// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "contour_types.hpp"
#include "phi_editor.hpp"
#include "point.hpp"

#include <QImage>
#include <QObject>
#include <QTimer>

#include <vector>

namespace fluvel_app
{

struct Span
{
    int y;
    int xLeft;
    int xRight;
};

class PhiViewModel : public QObject
{
    Q_OBJECT
public:
    explicit PhiViewModel(PhiEditor* editor, fluvel_ip::Connectivity connectivity,
                          QObject* parent = nullptr);

    const QImage& phi() const
    {
        return phi_;
    }

    void showOverlay();
    void hideOverlay();

    void setOverlay(const ShapeInfo& overlayShape);
    void setConnectivity(fluvel_ip::Connectivity c);
    void setInteractiveMode(bool enabled);

public slots:
    void updateFromEditor();
    void onClearFromEditor();
    void setBackground(const QImage& image);

signals:
    void viewChanged(const QImage& image);

private:
    void updateLists();
    void updatePhiFromLists();
    void composeView();
    bool pointIsRedundant(int x, int y);

    PhiEditor* editor_; // non owning
    QImage background_;
    QImage phi_;
    QImage displayedPhi_;

    std::vector<fluvel_ip::Point2D_i> outerBoundary_;
    std::vector<fluvel_ip::Point2D_i> innerBoundary_;

    QSize listsGridSize_;

    ShapeInfo overlayShape_;

    fluvel_ip::Connectivity connectivity_;

    bool interactiveMode_ = false;
    bool overlayVisible_ = false;
};

} // namespace fluvel_app
