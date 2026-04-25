// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"
#include "color_selector_widget.hpp"
#include "image_viewer_listener.hpp"
#include "shape.hpp"

#include <QImage>
#include <QString>
#include <QWidget>

#include <random>

class QSpinBox;
class QLabel;
class QPushButton;
class QGroupBox;

namespace fluvel_app
{

class ImageViewerWidget;

class AnalysisWidget : public QWidget, public ImageViewerListener
{
    Q_OBJECT

public:
    AnalysisWidget(QWidget* parent = nullptr);

    [[nodiscard]] int imageWidth() const
    {
        return imageWidth_;
    }

    [[nodiscard]] int imageHeight() const
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
        return selectedColor_;
    }

    void saveSettings() const;

signals:
    void listChanged();

private:
    void openFilename();
    void openImage();
    void refresh();
    void refreshWithNoise(int noisePercent);
    void onColorPicked(const QColor& color, const QPoint& /*imagePos*/) override;
    void onColorSelected(const QColor& c);

    void createList();

    QLabel* textListLength_ = nullptr;
    QString absoluteName_;
    QLabel* nameLabel_ = nullptr;
    ImageViewerWidget* imageViewer_ = nullptr;
    QPushButton* openButton_ = nullptr;

    ColorSelectorWidget* colorSelector_ = nullptr;
    fluvel_ip::Rgb_uc selectedColor_;

    QGroupBox* noiseGroup_ = nullptr;
    QSpinBox* noiseSp_ = nullptr;
    std::mt19937 rng_{std::random_device{}()};
    QImage noiseImage_;

    QImage image_;
    int imageWidth_{0};
    int imageHeight_{0};

    fluvel_ip::Shape shape_;

    QString lastDirectoryUsed_;
    QStringList nameFilters_;

    static int countThis;
    int idThis_;
};

} // namespace fluvel_app
