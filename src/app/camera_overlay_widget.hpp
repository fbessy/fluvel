#ifndef CAMERA_OVERLAY_WIDGET_HPP
#define CAMERA_OVERLAY_WIDGET_HPP


#include <QWidget>
#include <QLabel>

namespace ofeli_gui {

struct CameraStatsUi
{
    float inputFps = 0.0;
    float processingFps = 0.0;
    float displayFps = 0.0;
    float dropRate = 0.0;
    float avgLatencyMs = 0.0;
    float maxLatencyMs = 0.0;
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
