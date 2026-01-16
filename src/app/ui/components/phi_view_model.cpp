#include "phi_view_model.hpp"
#include "phi_editor.hpp"

#include <QPainter>

namespace ofeli_app {

PhiViewModel::PhiViewModel(PhiEditor* editor,
                           QObject* parent)
    : QObject(parent),
    editor_(editor)
{
    Q_ASSERT(editor_);

    background_ = QImage(editor_->phi().width(),
                         editor_->phi().height(),
                         QImage::Format_RGB32);

    background_.fill(Qt::black);

    updateLists();
    updatePhiFromLists();

    connect(editor_, &PhiEditor::phiChanged,
            this, &PhiViewModel::updateFromEditor);

    connect(editor_, &PhiEditor::phiCleared,
            this, &PhiViewModel::onClearFromEditor);

    connect(editor_, &PhiEditor::phiResized,
            this, &PhiViewModel::onPhiResized);

    updateTimer_.setSingleShot(true);
    updateTimer_.setTimerType(Qt::CoarseTimer);

    connect(&updateTimer_, &QTimer::timeout, this, [this]()
            {
                updateListsFloodFill();
                updatePhiFromLists();
                composeView(false);
            });
}

// called after an image reload with size change
// and juste before the callback onPhiResized
void PhiViewModel::setBackground(const QImage& image)
{
    background_ = image.convertToFormat(QImage::Format_RGB32);
}

// called after an image reload without size change
void PhiViewModel::setBackgroundWithUpdate(const QImage& image)
{
    background_ = image.convertToFormat(QImage::Format_RGB32);

    updatePhiFromLists();
    composeView(true);
}

void PhiViewModel::onPhiResized(int width, int height)
{
    assert( width  == background_.width() &&
            height == background_.height() );


    updateTimer_.start(0);
}

void PhiViewModel::updateFromEditor()
{
    updateTimer_.start(0);
}

void PhiViewModel::onClearFromEditor()
{
    l_out.clear();
    l_in.clear();
    phiImage_ = background_;

    composeView(true);
}

void PhiViewModel::updateLists()
{
    if (!editor_)
        return;

    const int w = editor_->phi().width();
    const int h = editor_->phi().height();

    l_out.clear();
    l_in.clear();

    const auto phi = editor_->phi();

    for ( int y = 0; y < h; ++y )
    {
        const uchar* line = phi.constScanLine(y);

        for ( int x = 0; x < w; ++x )
        {
            uchar I = line[x];

            if ( !editor_->is_redundant(x, y) )
            {
                if ( I == 0 )
                {
                    l_out.emplace_back(x, y);
                }
                else if (I == 255)
                {
                    l_in.emplace_back(x, y);
                }
            }
        }
    }
}

void PhiViewModel::updateListsFloodFill()
{
    l_out.clear();
    l_in.clear();

    const QImage& phi = editor_->phi();
    const int w = phi.width();
    const int h = phi.height();

    Q_ASSERT(phi.format() == QImage::Format_Grayscale8);

    // Marqueur de pixels visités
    QImage visited(w, h, QImage::Format_Grayscale8);
    visited.fill(0);

    auto inside = [&](int x, int y) {
        return x >= 0 && x < w && y >= 0 && y < h;
    };

    auto pixel = [&](int x, int y) -> uchar {
        return phi.constScanLine(y)[x];
    };

    std::vector<Span> stack;
    stack.reserve(1024);

    for (int y = 0; y < h; ++y)
    {
        const uchar* line = phi.constScanLine(y);
        uchar* visitedLine = visited.scanLine(y);

        for (int x = 0; x < w; ++x)
        {
            if (visitedLine[x])
                continue;

            if (editor_->is_redundant(x, y))
                continue;

            uchar value = line[x];
            if (value != 0 && value != 255)
                continue;

            // Nouveau seed
            stack.push_back({ y, x, x });

            while (!stack.empty())
            {
                Span s = stack.back();
                stack.pop_back();

                int xl = s.xLeft;
                int xr = s.xRight;

                // Étendre à gauche
                while (xl - 1 >= 0 &&
                       !visitedLine[xl - 1] &&
                       !editor_->is_redundant(xl - 1, s.y) &&
                       pixel(xl - 1, s.y) == value)
                {
                    --xl;
                }

                // Étendre à droite
                while (xr + 1 < w &&
                       !visitedLine[xr + 1] &&
                       !editor_->is_redundant(xr + 1, s.y) &&
                       pixel(xr + 1, s.y) == value)
                {
                    ++xr;
                }

                // Marquer et stocker
                for (int xi = xl; xi <= xr; ++xi)
                {
                    visited.scanLine(s.y)[xi] = 1;

                    if (value == 0)
                        l_out.emplace_back(xi, s.y);
                    else
                        l_in.emplace_back(xi, s.y);
                }

                // Examiner lignes au-dessus et en dessous
                for (int ny : { s.y - 1, s.y + 1 })
                {
                    if (ny < 0 || ny >= h)
                        continue;

                    const uchar* nline = phi.constScanLine(ny);
                    uchar* vline = visited.scanLine(ny);

                    int xscan = xl;
                    while (xscan <= xr)
                    {
                        if (!vline[xscan] &&
                            !editor_->is_redundant(xscan, ny) &&
                            nline[xscan] == value)
                        {
                            int xstart = xscan;
                            while (xscan + 1 <= xr &&
                                   !vline[xscan + 1] &&
                                   !editor_->is_redundant(xscan + 1, ny) &&
                                   nline[xscan + 1] == value)
                            {
                                ++xscan;
                            }

                            stack.push_back({ ny, xstart, xscan });
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
    phiImage_ = background_;

    for( const auto& p : l_out )
    {
        QPoint point(p.x, p.y);
        phiImage_.setPixel(point, qRgb(0, 0, 255));
    }

    for( const auto& p : l_in )
    {
        QPoint point(p.x, p.y);
        phiImage_.setPixel(point, qRgb(255, 0, 0));
    }
}

void PhiViewModel::composeView(bool hasOverlay)
{
    QImage withOverlay = phiImage_;

    if (hasOverlay)
    {
        QPainter p(&withOverlay);
        p.setRenderHint(QPainter::Antialiasing, false);
        p.setPen(QPen(Qt::cyan, 2));
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

} // namespace ofeli_app
