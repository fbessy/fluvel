#ifndef PHI_VIEW_MODEL_HPP
#define PHI_VIEW_MODEL_HPP

#include <QObject>
#include <QImage>

#include <vector>

#include "phi_editor.hpp"   // pour PhiEditor
#include "matrix.hpp"       // ofeli_ip::Matrix
#include "point.hpp"

namespace ofeli_app {

class PhiViewModel : public QObject
{
    Q_OBJECT
public:
    explicit PhiViewModel(PhiEditor* editor,
                          QObject* parent = nullptr);

    const QImage& phiImage() const { return phiImage_; }
    void setOverlay(const ShapeInfo& overlayShape);

public slots:
    void onPhiResized(int width, int height);
    void updateFromEditor();
    void onClearFromEditor();
    void setBackgroundWithUpdate(const QImage& image);
    void setBackground(const QImage& image);

signals:
    void viewChanged(const QImage& image);

private:

    void updateLists();
    void updatePhiFromLists();
    void composeView(bool hasOverlay);

    PhiEditor* editor_;   // non owning
    QImage phiImage_;
    QImage background_;

    std::vector<ofeli_ip::Point_i> l_out;
    std::vector<ofeli_ip::Point_i> l_in;

    ShapeInfo overlayShape_;
};

} // namespace ofeli_app

#endif // PHI_VIEW_MODEL_HPP
