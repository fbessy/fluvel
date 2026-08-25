// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "video_types.hpp"

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QObject>

class QVideoSink;

namespace fluvel
{

/**
 * @brief Manages a media video source.
 *
 * MediaVideoSource encapsulates the Qt Multimedia media player used to
 * open and play a media source.
 *
 * The class owns the media player and uses the audio output supplied by
 * the caller. The video sink is also supplied by the caller and is not
 * owned by this class.
 *
 * Media-specific errors and media status changes are reported through
 * signals. The caller remains responsible for deciding how these events
 * affect the application streaming state.
 *
 * Starting is asynchronous; playback and backend errors are reported
 * through the corresponding signals.
 */
class MediaVideoSource : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a media video source.
     *
     * @param audioOutput Audio output used for media playback.
     * @param parent Optional QObject parent.
     */
    explicit MediaVideoSource(QAudioOutput* audioOutput, QObject* parent = nullptr);

    /**
     * @brief Destroys the media video source.
     */
    ~MediaVideoSource() override;

    /**
     * @brief Starts the requested media source.
     *
     * Opens the specified source and starts playback.
     *
     * @param config Media source configuration.
     *
     * @return true if the source was accepted for playback.
     */
    bool start(const MediaSourceConfig& config);

    /**
     * @brief Stops the active media source.
     *
     * Stops playback, detaches the video sink and clears the current source.
     */
    void stop();

    /**
     * @brief Sets the video sink receiving frames from the media player.
     *
     * The sink is not owned by MediaVideoSource.
     *
     * @param sink Video sink to receive media frames, or nullptr to detach it.
     */
    void setVideoSink(QVideoSink* sink);

    /**
     * @brief Returns information about the active media source.
     */
    [[nodiscard]] MediaSourceInfo sourceInfo() const;

    /**
     * @brief Returns metadata about the current media.
     */
    [[nodiscard]] MediaInfo mediaInfo() const;

    /**
     * @brief Returns the current playback position.
     */
    [[nodiscard]] qint64 positionMs() const;

    /**
     * @brief Returns the current media duration.
     */
    [[nodiscard]] qint64 durationMs() const;

    /**
     * @brief Changes the playback position.
     *
     * @param positionMs Requested position in milliseconds.
     */
    void setPosition(qint64 positionMs);

    /**
     * @brief Returns whether the media is currently paused.
     */
    [[nodiscard]] bool isPaused() const;

    /**
     * @brief Pauses playback.
     */
    void pause();

    /**
     * @brief Resumes playback.
     */
    void resume();

    /**
     * @brief Returns whether a media source is currently active.
     */
    [[nodiscard]] bool isActive() const noexcept;

signals:
    /**
     * @brief Reports a playback position change.
     *
     * @param position Position in milliseconds.
     */
    void positionChanged(qint64 position);

    /**
     * @brief Reports a playback state change.
     *
     * @param state Current playback state.
     */
    void playbackStateChanged(QMediaPlayer::PlaybackState state);

    /**
     * @brief Reports an error from the media player backend.
     */
    void error(QMediaPlayer::Error error, const QString& errorString);

    /**
     * @brief Reports a media status change.
     */
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);

    /**
     * @brief Reports updated media metadata.
     */
    void mediaInfoChanged(const MediaInfo& info);

private:
    void updateMediaInfo();

    static bool isUsefulMediaTitle(const QString& title);

    QMediaPlayer mediaPlayer_;
    QAudioOutput* audioOutput_{nullptr};
    QVideoSink* videoSink_{nullptr};

    MediaSourceInfo sourceInfo_{};
    MediaInfo mediaInfo_{};
};

} // namespace fluvel
