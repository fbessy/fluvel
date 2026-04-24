// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"
#include "color_selector_widget.hpp"
#include "image_viewer_listener.hpp"
#include "shape.hpp"

#include <QString>
#include <QWidget>

class QSpinBox;
class QLabel;
class QPushButton;

namespace fluvel_app
{

class ImageViewerWidget;

class AnalysisWidget : public QWidget, public ImageViewerListener
{
    Q_OBJECT

public:
    AnalysisWidget(QWidget* parent = nullptr);

    int imageWidth() const
    {
        return imageWidth_;
    }
    int imageHeight() const
    {
        return imageHeight_;
    }

    const fluvel_ip::Shape& shape() const
    {
        return shape_;
    }
    const QImage& image() const
    {
        return image_;
    }
    const fluvel_ip::Rgb_uc& rgb() const
    {
        return rgb_;
    }

    void saveSettings() const;

private:
    void createList();

    QLabel* textListLength_;
    QString absoluteName_;
    QLabel* nameLabel_;
    ImageViewerWidget* imageViewer_;
    QPushButton* openButton_;

    ColorSelectorWidget* colorSelector_;
    fluvel_ip::Rgb_uc selected_;
    QSpinBox* noiseSp_;

    QImage image_;
    QImage noiseImage_;
    int imageWidth_{0};
    int imageHeight_{0};

    fluvel_ip::Shape shape_;
    fluvel_ip::Rgb_uc rgb_;

    QString lastDirectoryUsed_;
    QStringList nameFilters_;

    static int countThis;
    int idThis_;

private slots:

    void openFilename();
    void openImage();
    void refreshRgb();
    void refreshNoiseImage(int noise_percent);
    void onColorPicked(const QColor& color, const QPoint& /*imagePos*/) override;
    void onColorSelected(const QColor& c);

signals:

    void listChanged();
};

} // namespace fluvel_app
