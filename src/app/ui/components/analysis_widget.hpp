// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"
#include "image_viewer_listener.hpp"
#include "shape.hpp"

#include <QString>
#include <QWidget>

class QSpinBox;
class QComboBox;
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

    fluvel_ip::Shape& shape()
    {
        return shape_;
    }
    const QImage image() const
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

    QComboBox* colorList_;
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
    void getListColor();
    void refreshRgb(int);
    void refreshNoiseImage(int noise_percent);
    void onColorPicked(const QColor& color, const QPoint& /*imagePos*/) override;

signals:

    void listChanged();
};

} // namespace fluvel_app
