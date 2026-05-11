// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "phi_view_model.hpp"
#include "phi_editor.hpp"

#include <QPainter>

#ifdef FLUVEL_DEBUG
#include <QDebug>
#endif

namespace fluvel_app
{

// phi colors (Lout and Lin)
const QColor kOutColor{64, 0, 255};
const QColor kInColor{0, 230, 118};

// phi (Lout and Lin colors) in interactive mode
const QColor kInteractiveOuterColor{32, 0, 128};
const QColor kInteractiveInnerColor{0, 115, 59};

// overlay colors (user interaction)
const QColor kOverlayMainColor{kInColor};
const QColor kOverlayOutlineColor{kOutColor};

constexpr int kOverlayMainWidth{2};
constexpr int kOverlayOutlineWidth{4};

PhiViewModel::PhiViewModel(PhiEditor* editor, fluvel_ip::Connectivity connectivity, QObject* parent)
    : QObject(parent)
    , editor_(editor)
    , connectivity_(connectivity)
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

    overlayVisible_ = false;
    composeView();
}

void PhiViewModel::setBackground(const QImage& image)
{
    background_ = image.convertToFormat(QImage::Format_RGB32);

    updateLists();
    updatePhiFromLists();

    overlayVisible_ = false;
    composeView();
}

void PhiViewModel::updateFromEditor()
{
#ifdef FLUVEL_DEBUG
    qDebug() << __FILE__ << " updateFromEditor() " << __LINE__ << __func__;
#endif

    updateLists();
    updatePhiFromLists();

    overlayVisible_ = false;
    composeView();
}

void PhiViewModel::onClearFromEditor()
{
    outerBoundary_.clear();
    innerBoundary_.clear();
    displayedPhi_ = background_;

    overlayVisible_ = false;
    composeView();
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
    outerBoundary_.clear();
    innerBoundary_.clear();

    for (int y = 0; y < h; ++y)
    {
        const uchar* line = phi_.constScanLine(y);

        for (int x = 0; x < w; ++x)
        {
            uchar I = line[x];

            if (!pointIsRedundant(x, y))
            {
                if (I == 0)
                {
                    outerBoundary_.emplace_back(x, y);
                }
                else if (I == 255)
                {
                    innerBoundary_.emplace_back(x, y);
                }
            }
        }
    }
}

bool PhiViewModel::pointIsRedundant(int x, int y)
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

void PhiViewModel::updatePhiFromLists()
{
#ifdef FLUVEL_DEBUG
    qDebug() << __FILE__ << " updatePhiFromLists() " << __LINE__ << __func__;
#endif

    displayedPhi_ = background_;

    if (displayedPhi_.size() != listsGridSize_)
        return;

    Q_ASSERT(displayedPhi_.format() == QImage::Format_RGB32);

    const QRgb outColor = (interactiveMode_ ? kInteractiveOuterColor : kOutColor).rgb();
    const QRgb inColor = (interactiveMode_ ? kInteractiveInnerColor : kInColor).rgb();

    const int width = displayedPhi_.width();
    const int height = displayedPhi_.height();

    for (const auto& p : outerBoundary_)
    {
        if (p.y < 0 || p.y >= height || p.x < 0 || p.x >= width)
            continue;

        QRgb* line = reinterpret_cast<QRgb*>(displayedPhi_.scanLine(p.y));
        line[p.x] = outColor;
    }

    for (const auto& p : innerBoundary_)
    {
        if (p.y < 0 || p.y >= height || p.x < 0 || p.x >= width)
            continue;

        QRgb* line = reinterpret_cast<QRgb*>(displayedPhi_.scanLine(p.y));
        line[p.x] = inColor;
    }
}

void PhiViewModel::composeView()
{
#ifdef FLUVEL_DEBUG
    qDebug() << __FILE__ << " composeView() " << __LINE__ << __func__;
#endif

    QImage outputImg = displayedPhi_;

    if (overlayVisible_)
    {
        QPainter p(&outputImg);
        p.setRenderHint(QPainter::Antialiasing, false);
        p.setBrush(Qt::NoBrush);

        auto drawShape = [&](QPainter& painter)
        {
            if (overlayShape_.type == ShapeType::Rectangle)
                painter.drawRect(overlayShape_.boundingBox);
            else
                painter.drawEllipse(overlayShape_.boundingBox);
        };

        // --- outline (dessous) ---
        {
            QPen pen(kOverlayOutlineColor, kOverlayOutlineWidth);
            p.setPen(pen);
            drawShape(p);
        }

        // --- main (dessus) ---
        {
            QPen pen(kOverlayMainColor, kOverlayMainWidth);
            p.setPen(pen);
            drawShape(p);
        }
    }

    emit viewChanged(outputImg);
}

void PhiViewModel::setOverlay(const ShapeInfo& overlayShape)
{
    overlayShape_ = overlayShape;
    overlayVisible_ = true;
    composeView();
}

void PhiViewModel::showOverlay()
{
    overlayVisible_ = true;
    composeView();
}

void PhiViewModel::hideOverlay()
{
    overlayVisible_ = false;
    composeView();
}

void PhiViewModel::setInteractiveMode(bool enabled)
{
    interactiveMode_ = enabled;

    updatePhiFromLists(); // recalcul des couleurs atténuées
    composeView();
}

} // namespace fluvel_app
