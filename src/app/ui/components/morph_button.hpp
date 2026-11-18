// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "shape_morph.hpp"

#include <QList>
#include <QPushButton>

namespace fluvel
{

/**
 * @brief Push button whose symbol morphs between a list of shapes.
 *
 * The button knows nothing about what the shapes mean: the caller gives one
 * MorphShape per state with setShapes(), then selects the displayed one with
 * setShapeIndex(). The transition between two shapes is handled by ShapeMorph.
 *
 * There is no press feedback animation, so the morphing is the only motion.
 *
 * Example (three states):
 * @code
 * button->setShapes({{0.75, 0.5, {}},                 // disk, text color
 *                    {0.75, 0.15, Qt::red},           // rounded square
 *                    {0.45, 0.15, Qt::red}});         // small rounded square
 * button->setShapeIndex(1);
 * @endcode
 */
class MorphButton : public QPushButton
{
    Q_OBJECT

public:
    explicit MorphButton(QWidget* parent = nullptr);

    /**
     * @brief Sets the available shapes and displays the first one immediately.
     *
     * A shape with an invalid color is drawn with the text color of the
     * button palette.
     */
    void setShapes(const QList<MorphShape>& shapes);

    /**
     * @brief Morphs toward the shape at @p index (ignored if out of range).
     */
    void setShapeIndex(int index);

    [[nodiscard]]
    int shapeIndex() const;

    [[nodiscard]]
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent*) override;
    void changeEvent(QEvent* event) override;

private:
    /// Shape at @p index with the palette color applied when needed.
    [[nodiscard]]
    MorphShape resolvedShape(int index) const;

    QList<MorphShape> shapes_;
    int index_{0};

    ShapeMorph morph_;
};

} // namespace fluvel
