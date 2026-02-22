#ifndef PHI_VIEW_MODEL_HPP
#define PHI_VIEW_MODEL_HPP

#include <QObject>
#include <QImage>
#include <QTimer>

#include <vector>

#include "phi_editor.hpp"   // pour PhiEditor
#include "grid2d.hpp"       // ofeli_ip::Grid2D
#include "point.hpp"

namespace ofeli_app
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
    explicit PhiViewModel(PhiEditor* editor,
                          QObject* parent = nullptr);

    const QImage& phiImage() const { return displayedPhi_; }
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

    PhiEditor* editor_;   // non owning
    QImage displayedPhi_;
    QImage background_;

    std::vector<ofeli_ip::Point2D_i> l_out_;
    std::vector<ofeli_ip::Point2D_i> l_in_;

    QSize listsGridSize_;

    ShapeInfo overlayShape_;

    ofeli_ip::Connectivity connectivity_;
};

} // namespace ofeli_app

#endif // PHI_VIEW_MODEL_HPP
