#ifndef CAMERA_OVERLAY_WIDGET_HPP
#define CAMERA_OVERLAY_WIDGET_HPP


#include <QWidget>
#include <QLabel>

namespace ofeli_gui {

struct CameraStatsUi
{
    float inputFps = 0.f;
    float processingFps = 0.f;
    float displayFps = 0.f;
    float dropRate = 0.f;
    float avgLatencyMs = 0.f;
    float maxLatencyMs = 0.f;
};

class CameraOverlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraOverlayWidget(QWidget* parent = nullptr);

public slots:
    void setStats(const CameraStatsUi& stats);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QLabel* label_;
};

}

#endif // CAMERA_OVERLAY_WIDGET_HPP
