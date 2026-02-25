#ifndef CAMERA_OVERLAY_WIDGET_HPP
#define CAMERA_OVERLAY_WIDGET_HPP

#include <QLabel>
#include <QWidget>

#include "camera_stats.hpp"

namespace ofeli_app
{

class CameraOverlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraOverlayWidget(QWidget* parent = nullptr);

public slots:
    void setStats(const CameraStats& stats);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QLabel* label_;
};

} // namespace ofeli_app

#endif // CAMERA_OVERLAY_WIDGET_HPP
