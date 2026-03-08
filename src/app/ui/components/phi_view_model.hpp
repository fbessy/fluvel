// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "phi_editor.hpp"
#include "contour_data.hpp"
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
    explicit PhiViewModel(PhiEditor* editor, QObject* parent = nullptr);

    const QImage& phiImage() const
    {
        return displayedPhi_;
    }
    void setOverlay(const ShapeInfo& overlayShape);

public slots:
    void updateFromEditor();
    void onClearFromEditor();
    void setBackground(const QImage& image);
    void onConnectivityChanged(int index);

signals:
    void viewChanged(const QImage& image);

private:
    void updateLists();
    void updateListsFloodFill();
    void updatePhiFromLists();
    void composeView(bool hasOverlay);
    bool point_is_redundant(int x, int y);

    PhiEditor* editor_; // non owning
    QImage displayedPhi_;
    QImage background_;

    std::vector<fluvel_ip::Point2D_i> l_out_;
    std::vector<fluvel_ip::Point2D_i> l_in_;

    QSize listsGridSize_;

    ShapeInfo overlayShape_;

    fluvel_ip::Connectivity connectivity_;
};

} // namespace fluvel_app
