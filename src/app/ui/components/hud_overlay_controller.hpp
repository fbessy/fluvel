// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QObject>
#include <QPropertyAnimation>
#include <QTimer>

namespace fluvel
{

/**
 * @brief Predefined visual styles for HUD messages.
 */
enum class HudPreset
{
    /**
     * @brief Small overlay displayed near the mouse cursor.
     *
     * Intended for short contextual feedback, such as the current zoom level.
     */
    Cursor,

    /**
     * @brief Large centered notification overlay.
     *
     * Intended for keyboard actions and other transient user notifications.
     */
    Notification
};

class OverlayTextItem;

/**
 * @brief Controls the display and fade-out of temporary HUD messages.
 *
 * This class manages the lifecycle of transient messages displayed through an
 * OverlayTextItem.
 *
 * Depending on the selected preset, the controller configures the overlay
 * appearance (position, typography and animation timings), displays it for a
 * short duration, then smoothly fades it out.
 *
 * Internally uses a QTimer to control the display duration and a
 * QPropertyAnimation for the fade effect.
 *
 * @note The OverlayTextItem is not owned by this class. The caller must ensure
 * that it remains valid for the lifetime of the controller.
 */
class HudOverlayController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the controller.
     *
     * @param item Pointer to the overlay text item to control.
     * @param parent Optional QObject parent.
     *
     * @pre item must not be null.
     */
    explicit HudOverlayController(OverlayTextItem* item, QObject* parent = nullptr);

    /**
     * @brief Displays a temporary HUD message.
     *
     * Applies the specified visual preset, updates the displayed text and restarts
     * the display/fade sequence.
     *
     * @param text Text to display.
     * @param preset Visual preset controlling the appearance and behavior of the
     *               overlay.
     */
    void show(const QString& text, HudPreset preset = HudPreset::Cursor);

private slots:
    /**
     * @brief Called when the display timer expires.
     *
     * Starts the fade-out animation.
     */
    void onTimeout();

    /**
     * @brief Called when the fade-out animation finishes.
     *
     * Typically used to hide or reset the overlay item.
     */
    void onFadeFinished();

private:
    /**
     * @brief Applies one of the predefined HUD visual presets.
     *
     * Configures the overlay appearance and animation timings.
     *
     * @param preset Preset to apply.
     */
    void applyPreset(HudPreset preset);

    /**
     * @brief Starts the fade-out animation.
     *
     * Configures and launches the opacity animation on the overlay item.
     */
    void startFade();

    /// Overlay item displaying the zoom text (not owned).
    OverlayTextItem* item_{nullptr};

    /// Timer controlling how long the overlay stays visible.
    QTimer timer_;

    /// Animation used to fade out the overlay.
    QPropertyAnimation anim_;

    /// Duration (in ms) the overlay remains fully visible before fading.
    int displayDurationMs_{800};

    /// Duration (in ms) of the fade-out animation.
    int fadeDurationMs_{250};
};

} // namespace fluvel
