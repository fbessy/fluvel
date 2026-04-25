// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file phi_editor.hpp
 * @brief Interactive editor for level-set (phi) representation.
 *
 * This component provides editing capabilities for the level-set function
 * (phi) using shape-based operations. It maintains both a working copy
 * and a committed version to support undo/revert workflows.
 *
 * Typical usage:
 * - Load or initialize phi
 * - Apply shape operations (add/subtract)
 * - Preview changes
 * - Commit or revert edits
 */

#pragma once

#include "shape_type.hpp"

#include <QObject>
#include <QImage>
#include <QRect>

namespace fluvel_app
{

/**
 * @brief Shape definition in image space.
 *
 * Describes a geometric primitive used to modify the phi mask.
 */
struct ShapeInfo
{
    ShapeType type;    ///< Shape type (e.g. rectangle, ellipse)
    QRect boundingBox; ///< Bounding box in image coordinates
};

/**
 * @brief Editor for level-set image (phi).
 *
 * Maintains two versions of phi:
 * - editedPhi_   : working copy (modifiable)
 * - committedPhi_: last committed state
 *
 * Provides shape-based editing operations and supports commit/revert semantics.
 *
 * @note Intended for use in the Qt main thread.
 */
class PhiEditor : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct editor with initial phi image.
     */
    PhiEditor(const QImage& phi);

    /**
     * @brief Load a new phi image.
     *      * Replaces both edited and committed states.
     */
    void loadPhi(const QImage& phi);

    /**
     * @brief Add a shape to the phi mask.
     *      * Typically marks pixels inside the shape as active.
     */
    void addShape(const ShapeInfo& shape);

    /**
     * @brief Subtract a shape from the phi mask.
     *      * Typically removes pixels inside the shape from the active region.
     */
    void subtractShape(const ShapeInfo& shape);

    /**
     * @brief Clear the phi mask.
     */
    void clear();

    /**
     * @brief Commit current edits.
     *      * Copies the edited state into the committed state.
     *      * @return The committed phi image.
     */
    QImage commit();

    /**
     * @brief Revert edits to last committed state.
     */
    void revert();

    /**
     * @brief Resize the phi image.
     *      * @param size New image size.
     */
    void setSize(const QSize& size);

    /**
     * @brief Access current edited phi.
     *      * @return Reference to working phi image.
     */
    const QImage& phi() const
    {
        return editedPhi_;
    }

signals:
    /// Emitted when the edited phi is modified.
    void editedPhiChanged();

    /// Emitted when the phi is cleared.
    void editedPhiCleared();

private:
    /**
     * @brief Apply a shape modification to phi.
     *      * @param shapeInfo Shape definition.
     * @param color Value used to update the phi image.
     */
    void changeShape(const ShapeInfo& shapeInfo, const QColor& color);

    /// Working (editable) phi image.
    QImage editedPhi_;

    /// Last committed phi image.
    QImage committedPhi_;
};

} // namespace fluvel_app
