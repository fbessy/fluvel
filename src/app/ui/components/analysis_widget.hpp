// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "color.hpp"
#include "image_viewer_listener.hpp"
#include "shape.hpp"

#include <QWidget>

QT_BEGIN_NAMESPACE
class QSpinBox;
class QComboBox;
class QMimeData;
class QLabel;
class QPushButton;
QT_END_NAMESPACE

namespace fluvel_app
{

class ImageViewerWidget;

class AnalysisWidget : public QWidget, public ImageViewerListener
{
    Q_OBJECT

public:
    AnalysisWidget(QWidget* parent = nullptr);

    int get_img_width() const
    {
        return img_width_;
    }
    int get_img_height() const
    {
        return img_height_;
    }

    fluvel_ip::Shape& get_shape()
    {
        return shape_;
    }
    const QImage get_image() const
    {
        return img_;
    }
    const fluvel_ip::Rgb_uc& get_rgb() const
    {
        return rgb_;
    }

    void save_settings() const;

private:
    void create_list();

    QLabel* text_list_length_;
    QString absolute_name_;
    QLabel* name_label_;
    ImageViewerWidget* imageViewer_;
    QPushButton* open_button_;

    QComboBox* color_list_;
    fluvel_ip::Rgb_uc selected_;
    QSpinBox* noise_sp_;

    QImage img_;
    QImage img_noise_;
    int img_width_{0};
    int img_height_{0};

    fluvel_ip::Shape shape_;
    fluvel_ip::Rgb_uc rgb_;

    QString last_directory_used_;
    QStringList name_filters_;

    static int count_this;
    int id_this_;

private slots:

    void open_filename();
    void open_img();
    void get_list_color();
    void refresh_rgb(int);
    void refresh_img_noise(int noise_percent);
    void onColorPicked(const QColor& color, const QPoint& /*imagePos*/) override;

signals:

    void change_list();
};

} // namespace fluvel_app
