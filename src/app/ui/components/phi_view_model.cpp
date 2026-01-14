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

    connect(editor_, &PhiEditor::phiChanged,
            this, &PhiViewModel::updateFromEditor);

    updateFromEditor();
    composeView(true);
}

void PhiViewModel::setBackground(const QImage& image)
{
    background_ = image.convertToFormat(QImage::Format_RGB32);

    updateFromEditor();
    composeView(true);
}

void PhiViewModel::updateFromEditor()
{
    if (!editor_)
        return;

    phiImage_ = background_;

    const int w = editor_->get_phi().width();
    const int h = editor_->get_phi().height();

    for (int x = 0; x < w; ++x)
    {
        for (int y = 0; y < h; ++y)
        {
            if ( !editor_->is_redundant(x, y) )
            {
                QPoint p(x, y);

                if ( qGray(editor_->get_phi().pixel(x, y)) == 0 )
                {
                    phiImage_.setPixel(p, qRgb(0, 0, 255));
                }
                else if ( qGray(editor_->get_phi().pixel(x, y)) == 255 )
                {
                    phiImage_.setPixel(p, qRgb(255, 0, 0));
                }
            }
        }
    }

    composeView(false);
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
