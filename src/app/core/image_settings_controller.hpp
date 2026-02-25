#ifndef IMAGE_SETTINGS_CONTROLLER_HPP
#define IMAGE_SETTINGS_CONTROLLER_HPP

#include <QObject>

#include "phi_editor.hpp"
#include "phi_view_model.hpp"

namespace ofeli_app
{

struct UiShapeInfo
{
    ShapeType shape;
    int width;
    int height;
    int x;
    int y;
};

class ImageSettingsController : public QObject
{
    Q_OBJECT

public:
    ImageSettingsController(QObject* parent);

    void addShape(UiShapeInfo uiShape);
    void subtractShape(UiShapeInfo uiShape);
    void clearPhi();
    void onInputImageReady(const QImage& inputImage);

    void accept();
    void reject();

public slots:
    void onUpdateOverlay(UiShapeInfo uiShape);
    void onViewChanged(const QImage& imageSettings);

signals:
    void viewChanged(const QImage& imageSettings);

private:
    void setInitialPhi(const QImage& phi);

    ShapeInfo computeShapeInfo(const UiShapeInfo& uiShape);

    std::unique_ptr<PhiEditor> phiEditor_;
    std::unique_ptr<PhiViewModel> phiViewModel_;
};

} // namespace ofeli_app

#endif // IMAGE_SETTINGS_CONTROLLER_HPP
