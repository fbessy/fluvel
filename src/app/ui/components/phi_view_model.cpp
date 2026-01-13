#include "phi_view_model.hpp"
#include "phi_editor.hpp"

namespace ofeli_app {

PhiViewModel::PhiViewModel(PhiEditor* editor, QObject* parent)
    : QObject(parent),
      editor_(editor)
{
    Q_ASSERT(editor_);

    connect(editor_, &PhiEditor::phiChanged,
            this, &PhiViewModel::updateFromEditor);

    updateFromEditor();
}

void PhiViewModel::updateFromEditor()
{
    if (!editor_)
        return;

    phiImage_ = editor_->get_phi().copy();
    background_ = editor_->get_phi().copy();

    emit viewChanged();
}

} // namespace ofeli_app
