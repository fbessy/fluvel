#include "camera_overlay_widget.hpp"

#include <QVBoxLayout>

namespace ofeli_app
{

CameraOverlayWidget::CameraOverlayWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    label_ = new QLabel(this);
    label_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    label_->setWordWrap(true);

    label_->setStyleSheet("color: white;"
                          "background: rgba(0, 0, 0, 140);"
                          "padding: 4px 6px;"
                          "border-radius: 4px;");

    label_->adjustSize();

    layout->addWidget(label_);
}

void CameraOverlayWidget::resizeEvent(QResizeEvent*)
{
    // coin haut-gauche
    label_->move(10, 10);
}

void CameraOverlayWidget::setStats(const CameraStats& s)
{
    label_->setText(QString("In: %1 fps | Proc: %2 fps | Disp: %3 fps\n"
                            "Drop: %4 % | Avg: %5 ms | Max: %6 ms")
                        .arg(s.inputFps, 0, 'f', 1)
                        .arg(s.processingFps, 0, 'f', 1)
                        .arg(s.displayFps, 0, 'f', 1)
                        .arg(100.f * s.dropRate, 0, 'f', 1)
                        .arg(s.avgLatencyMs, 0, 'f', 1)
                        .arg(s.maxLatencyMs, 0, 'f', 1));

    label_->adjustSize();
}

} // namespace ofeli_app
