#ifndef PHI_VIEW_MODEL_HPP
#define PHI_VIEW_MODEL_HPP

#include <QObject>
#include <QImage>

#include "phi_editor.hpp"   // pour PhiEditor
#include "matrix.hpp"       // ofeli_ip::Matrix

namespace ofeli_app {

class PhiViewModel : public QObject
{
    Q_OBJECT
public:
    explicit PhiViewModel(PhiEditor* editor, QObject* parent = nullptr);

    const QImage& phiImage() const { return phiImage_; }

public slots:
    void updateFromEditor();

signals:
    void viewChanged();

private:
    PhiEditor* editor_;   // non owning
    QImage phiImage_;
    QImage background_;
};

} // namespace ofeli_app

#endif // PHI_VIEW_MODEL_HPP
