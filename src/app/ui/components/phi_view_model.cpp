#include "phi_view_model.hpp"
#include "phi_editor.hpp"

namespace ofeli_app {

PhiViewModel::PhiViewModel(PhiEditor* editor, QObject* parent)
    : QObject(parent),
      editor_(editor)
{
    Q_ASSERT(editor_);

    connect(editor_, &PhiEditor::phiChanged,
            this, &PhiViewModel::rebuild);

    rebuild();
}

void PhiViewModel::rebuild()
{
    const auto& phi = editor_->phi();

    const int w = phi.get_width();
    const int h = phi.get_height();

    if (w <= 0 || h <= 0)
        return;

    phiImage_ = QImage(w, h, QImage::Format_RGB32);

    for (int y = 0; y < h; ++y) {
        uchar* line = phiImage_.scanLine(y);
        for (int x = 0; x < w; ++x) {
            const bool inside =
                (phi(x, y) == ofeli_ip::PhiValue::INSIDE_REGION);

            const uchar v = inside ? 255 : 0;

            line[4*x + 0] = v;
            line[4*x + 1] = v;
            line[4*x + 2] = v;
            line[4*x + 3] = 255;
        }
    }

    emit viewChanged();
}

} // namespace ofeli_app
