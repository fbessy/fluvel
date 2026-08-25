// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "media_video_source.hpp"

#include <QMediaMetaData>
#include <QVideoSink>

#include <algorithm>

namespace fluvel
{

MediaVideoSource::MediaVideoSource(QAudioOutput* audioOutput, QObject* parent)
    : QObject(parent)
    , audioOutput_(audioOutput)
{
    Q_ASSERT(audioOutput_);

    mediaPlayer_.setAudioOutput(audioOutput_);

    connect(&mediaPlayer_, &QMediaPlayer::errorOccurred, this, &MediaVideoSource::error);

    connect(&mediaPlayer_, &QMediaPlayer::mediaStatusChanged, this,
            [this](QMediaPlayer::MediaStatus status)
            {
                updateMediaInfo();
                emit mediaStatusChanged(status);
            });

    connect(&mediaPlayer_, &QMediaPlayer::positionChanged, this,
            &MediaVideoSource::positionChanged);

    connect(&mediaPlayer_, &QMediaPlayer::metaDataChanged, this,
            &MediaVideoSource::updateMediaInfo);

    connect(&mediaPlayer_, &QMediaPlayer::playbackStateChanged, this,
            &MediaVideoSource::playbackStateChanged);
}

MediaVideoSource::~MediaVideoSource()
{
    stop();
}

bool MediaVideoSource::start(const MediaSourceConfig& config)
{
    if (isActive())
        return false;

    sourceInfo_ = {};
    sourceInfo_.sourceUrl = config.sourceUrl;

    mediaInfo_ = {};

    mediaPlayer_.setVideoSink(videoSink_);
    mediaPlayer_.setSource(config.sourceUrl);
    mediaPlayer_.play();

    return true;
}

void MediaVideoSource::stop()
{
    if (!isActive())
        return;

    mediaPlayer_.stop();
    mediaPlayer_.setSource({});
    mediaPlayer_.setVideoSink(nullptr);

    sourceInfo_ = {};
    mediaInfo_ = {};
}

void MediaVideoSource::setVideoSink(QVideoSink* sink)
{
    videoSink_ = sink;
    mediaPlayer_.setVideoSink(sink);
}

MediaSourceInfo MediaVideoSource::sourceInfo() const
{
    return sourceInfo_;
}

MediaInfo MediaVideoSource::mediaInfo() const
{
    return mediaInfo_;
}

qint64 MediaVideoSource::positionMs() const
{
    return mediaPlayer_.position();
}

qint64 MediaVideoSource::durationMs() const
{
    return mediaPlayer_.duration();
}

void MediaVideoSource::setPosition(qint64 positionMs)
{
    positionMs = std::clamp(positionMs, 0LL, durationMs());
    mediaPlayer_.setPosition(positionMs);
}

bool MediaVideoSource::isPaused() const
{
    return mediaPlayer_.playbackState() == QMediaPlayer::PausedState;
}

void MediaVideoSource::pause()
{
    mediaPlayer_.pause();
}

void MediaVideoSource::resume()
{
    mediaPlayer_.play();
}

bool MediaVideoSource::isActive() const noexcept
{
    return !sourceInfo_.sourceUrl.isEmpty();
}

void MediaVideoSource::updateMediaInfo()
{
    MediaInfo info{};

    info.seekable = mediaPlayer_.isSeekable();
    info.durationMs = mediaPlayer_.duration();
    info.hasAudio = mediaPlayer_.hasAudio();

    const QString title = mediaPlayer_.metaData().stringValue(QMediaMetaData::Title);

    if (isUsefulMediaTitle(title))
        info.title = title;

    const double fps = mediaPlayer_.metaData().value(QMediaMetaData::VideoFrameRate).toDouble();

    if (fps > 0.0)
        info.frameRate = fps;

    mediaInfo_ = info;

    emit mediaInfoChanged(mediaInfo_);
}

bool MediaVideoSource::isUsefulMediaTitle(const QString& title)
{
    const QString trimmed = title.trimmed();

    if (trimmed.isEmpty())
        return false;

    static const QStringList kIgnoredTitles{"video",    "track",   "track 1",
                                            "untitled", "unknown", "media"};

    return !kIgnoredTitles.contains(trimmed.toLower());
}

} // namespace fluvel
