// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "phi_view_model.hpp"
#include "phi_editor.hpp"

#include <QPainter>

namespace fluvel_app
{

PhiViewModel::PhiViewModel(PhiEditor* editor, QObject* parent)
    : QObject(parent)
    , editor_(editor)
{
    Q_ASSERT(editor_);

    background_ = QImage(editor_->phi().width(), editor_->phi().height(), QImage::Format_RGB32);

    background_.fill(Qt::black);

    phi_ = editor_->phi();

    updateLists();
    updatePhiFromLists();

    connect(editor_, &PhiEditor::editedPhiChanged, this, &PhiViewModel::updateFromEditor);

    connect(editor_, &PhiEditor::editedPhiCleared, this, &PhiViewModel::onClearFromEditor);
}

void PhiViewModel::setConnectivity(fluvel_ip::Connectivity c)
{
    connectivity_ = c;

    updateLists();
    updatePhiFromLists();
    composeView(true);
}

void PhiViewModel::setBackground(const QImage& image)
{
    background_ = image.convertToFormat(QImage::Format_RGB32);

    updateListsFloodFill();
    updatePhiFromLists();
    composeView(true);
}

void PhiViewModel::updateFromEditor()
{
    updateListsFloodFill();
    updatePhiFromLists();
    composeView(false);
}

void PhiViewModel::onClearFromEditor()
{
    l_out_.clear();
    l_in_.clear();
    displayedPhi_ = background_;

    composeView(true);
}

void PhiViewModel::updateLists()
{
    if (!editor_)
        return;

    phi_ = editor_->phi();

    if (phi_.size() != background_.size())
    {
        phi_ = phi_.scaled(background_.size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }

    const int w = phi_.width();
    const int h = phi_.height();

    listsGridSize_ = phi_.size();
    l_out_.clear();
    l_in_.clear();

    for (int y = 0; y < h; ++y)
    {
        const uchar* line = phi_.constScanLine(y);

        for (int x = 0; x < w; ++x)
        {
            uchar I = line[x];

            if (!point_is_redundant(x, y))
            {
                if (I == 0)
                {
                    l_out_.emplace_back(x, y);
                }
                else if (I == 255)
                {
                    l_in_.emplace_back(x, y);
                }
            }
        }
    }
}

bool PhiViewModel::point_is_redundant(int x, int y)
{
    assert(!phi_.isNull());

    const int w = phi_.width();
    const int h = phi_.height();

    const uchar center = phi_.constScanLine(y)[x];

    auto same_value = [&](int nx, int ny) -> bool
    {
        if (nx < 0 || nx >= w || ny < 0 || ny >= h)
            return true; // hors image = neutre

        return phi_.constScanLine(ny)[nx] == center;
    };

    // 4-connectivity
    // connectivity_ == Connectivity::Four
    if (connectivity_ == fluvel_ip::Connectivity::Four)
    {
        return same_value(x - 1, y) && same_value(x + 1, y) && same_value(x, y - 1) &&
               same_value(x, y + 1);
    }

    // 8-connectivity
    for (int dy = -1; dy <= 1; ++dy)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            if (dx == 0 && dy == 0)
                continue;

            if (!same_value(x + dx, y + dy))
                return false;
        }
    }

    return true;
}

void PhiViewModel::updateListsFloodFill()
{
    phi_ = editor_->phi();

    if (phi_.size() != background_.size())
    {
        phi_ = phi_.scaled(background_.size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }

    const int w = phi_.width();
    const int h = phi_.height();

    listsGridSize_ = phi_.size();
    l_out_.clear();
    l_in_.clear();

    Q_ASSERT(phi_.format() == QImage::Format_Grayscale8);

    // Marqueur de pixels visités
    QImage visited(w, h, QImage::Format_Grayscale8);
    visited.fill(0);

    /*
    auto inside = [&](int x, int y) {
        return x >= 0 && x < w && y >= 0 && y < h;
    };*/

    auto pixel = [&](int x, int y) -> uchar
    {
        return phi_.constScanLine(y)[x];
    };

    std::vector<Span> stack;
    stack.reserve(1024);

    for (int y = 0; y < h; ++y)
    {
        const uchar* line = phi_.constScanLine(y);
        uchar* visitedLine = visited.scanLine(y);

        for (int x = 0; x < w; ++x)
        {
            if (visitedLine[x])
                continue;

            if (point_is_redundant(x, y))
                continue;

            uchar value = line[x];
            if (value != 0 && value != 255)
                continue;

            // Nouveau seed
            stack.push_back({y, x, x});

            while (!stack.empty())
            {
                Span s = stack.back();
                stack.pop_back();

                int xl = s.xLeft;
                int xr = s.xRight;

                // Étendre à gauche
                while (xl - 1 >= 0 && !visitedLine[xl - 1] && !point_is_redundant(xl - 1, s.y) &&
                       pixel(xl - 1, s.y) == value)
                {
                    --xl;
                }

                // Étendre à droite
                while (xr + 1 < w && !visitedLine[xr + 1] && !point_is_redundant(xr + 1, s.y) &&
                       pixel(xr + 1, s.y) == value)
                {
                    ++xr;
                }

                // Marquer et stocker
                for (int xi = xl; xi <= xr; ++xi)
                {
                    visited.scanLine(s.y)[xi] = 1;

                    if (value == 0)
                        l_out_.emplace_back(xi, s.y);
                    else
                        l_in_.emplace_back(xi, s.y);
                }

                // Examiner lignes au-dessus et en dessous
                for (int ny : {s.y - 1, s.y + 1})
                {
                    if (ny < 0 || ny >= h)
                        continue;

                    const uchar* nline = phi_.constScanLine(ny);
                    uchar* vline = visited.scanLine(ny);

                    int xscan = xl;
                    while (xscan <= xr)
                    {
                        if (!vline[xscan] && !point_is_redundant(xscan, ny) &&
                            nline[xscan] == value)
                        {
                            int xstart = xscan;
                            while (xscan + 1 <= xr && !vline[xscan + 1] &&
                                   !point_is_redundant(xscan + 1, ny) && nline[xscan + 1] == value)
                            {
                                ++xscan;
                            }

                            stack.push_back({ny, xstart, xscan});
                        }
                        ++xscan;
                    }
                }
            }
        }
    }
}

void PhiViewModel::updatePhiFromLists()
{
    displayedPhi_ = background_;

    if (displayedPhi_.size() != listsGridSize_)
        return;

    for (const auto& p : l_out_)
    {
        QPoint point(p.x, p.y);
        displayedPhi_.setPixel(point, qRgb(0, 0, 255));
    }

    for (const auto& p : l_in_)
    {
        QPoint point(p.x, p.y);
        displayedPhi_.setPixel(point, qRgb(255, 0, 0));
    }
}

void PhiViewModel::composeView(bool hasOverlay)
{
    QImage withOverlay = displayedPhi_;

    if (hasOverlay)
    {
        QPainter p(&withOverlay);
        // p.setRenderHint(QPainter::Antialiasing, false);

        QColor overlayColor = Qt::green;
        overlayColor.setAlpha(150);

        QPen pen(overlayColor, 3);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);

        if (overlayShape_.type == ShapeType::Rectangle)
            p.drawRect(overlayShape_.boundingBox);
        else
            p.drawEllipse(overlayShape_.boundingBox);
    }

    emit viewChanged(withOverlay);
}

void PhiViewModel::setOverlay(const ShapeInfo& overlayShape)
{
    overlayShape_ = overlayShape;
    composeView(true);
}

} // namespace fluvel_app
