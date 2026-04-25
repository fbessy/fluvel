// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QObject>
#include <QPropertyAnimation>
#include <QTimer>

namespace fluvel_app
{

class OverlayTextItem;

/**
 * @brief Controls the display and fade-out of a zoom overlay text.
 *
 * This class manages the lifecycle of a transient overlay displaying
 * the current zoom level (e.g. "150%").
 *
 * When triggered via show(), the overlay is:
 * - immediately displayed
 * - kept visible for a short duration
 * - then smoothly faded out using a property animation
 *
 * Internally uses a QTimer to control the display duration and a
 * QPropertyAnimation for the fade effect.
 *
 * @note The OverlayTextItem is not owned by this class.
 *       The caller must ensure that it remains valid for the lifetime
 *       of the controller.
 */
class ZoomOverlayController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the controller.
     *      * @param item Pointer to the overlay text item to control.
     * @param parent Optional QObject parent.
     *      * @pre item must not be null.
     */
    explicit ZoomOverlayController(OverlayTextItem* item, QObject* parent = nullptr);

    /**
     * @brief Displays the zoom overlay.
     *      * Updates the overlay text with the given zoom percentage and
     * restarts the display/fade sequence.
     *      * @param percent Zoom value in percent (e.g. 150 for 150%).
     */
    void show(int percent);

private slots:
    /**
     * @brief Called when the display timer expires.
     *      * Starts the fade-out animation.
     */
    void onTimeout();

    /**
     * @brief Called when the fade-out animation finishes.
     *      * Typically used to hide or reset the overlay item.
     */
    void onFadeFinished();

private:
    /**
     * @brief Starts the fade-out animation.
     *      * Configures and launches the opacity animation on the overlay item.
     */
    void startFade();

private:
    /// Overlay item displaying the zoom text (not owned).
    OverlayTextItem* item_ = nullptr;

    /// Timer controlling how long the overlay stays visible.
    QTimer timer_;

    /// Animation used to fade out the overlay.
    QPropertyAnimation* anim_ = nullptr;

    /// Duration (in ms) the overlay remains fully visible before fading.
    int displayDurationMs_{800};

    /// Duration (in ms) of the fade-out animation.
    int fadeDurationMs_{250};
};

} // namespace fluvel_app
