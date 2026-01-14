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

    background_ = QImage(editor_->get_phi().width(),
                         editor_->get_phi().height(),
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

    updateLists();
    updatePhiFromLists();
    composeView(false);
}

void PhiViewModel::updateFromEditor()
{
    updateLists();
    updatePhiFromLists();

    composeView(false);
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

    const int w = editor_->get_phi().width();
    const int h = editor_->get_phi().height();

    l_out.clear();
    l_in.clear();

    for (int x = 0; x < w; ++x)
    {
        for (int y = 0; y < h; ++y)
        {
            if ( !editor_->is_redundant(x, y) )
            {
                QPoint p(x, y);

                if ( qGray(editor_->get_phi().pixel(x, y)) == 0 )
                {
                    l_out.emplace_back(x,y);
                }
                else if ( qGray(editor_->get_phi().pixel(x, y)) == 255 )
                {
                    l_in.emplace_back(x,y);
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
        p.setRenderHint(QPainter::Antialiasing, true);
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
