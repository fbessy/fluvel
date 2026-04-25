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

/**
 * @brief Widget for image analysis and shape extraction.
 *
 * This widget provides tools to:
 * - load and display images
 * - select colors for analysis
 * - generate and update shapes based on the selected color
 * - optionally apply noise for testing
 *
 * It integrates an ImageViewerWidget and reacts to user interactions
 * (e.g. color picking) via the ImageViewerListener interface.
 *
 * The widget maintains the current image, selected color, and resulting shape.
 */
class AnalysisWidget : public QWidget, public ImageViewerListener
{
    Q_OBJECT

public:
    AnalysisWidget(QWidget* parent = nullptr);

    /**
     * @name Accessors
     * @brief Read-only access to current analysis data.
     * @{
     */

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

    /** @} */

    /**
     * @brief Saves current widget settings.
     */
    void saveSettings() const;

signals:
    /**
     * @brief Emitted when the internal list or shape changes.
     */
    void listChanged();

private:
    /**
     * @brief Handles color picking events from the image viewer.
     *      * Updates the selected color and refreshes the analysis.
     */
    void onColorPicked(const QColor& color, const QPoint& imagePos) override;

    /// Opens a file dialog to select an image.
    void openFilename();

    /// Loads the selected image.
    void openImage();

    /// Refreshes the analysis.
    void refresh();

    /// Refreshes the analysis with noise applied.
    void refreshWithNoise(int noisePercent);

    /// Handles color selection from the UI widget.
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
