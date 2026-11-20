// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "ffmpeg_remux.hpp"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QStorageInfo>

#include <filesystem>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace fluvel::ffmpeg_utils
{

namespace
{

// Extra free space required on top of the file size.
constexpr qint64 kSpaceMarginBytes = 16 * 1024 * 1024;

bool hasEnoughSpace(const QFileInfo& info)
{
    const QStorageInfo storage(info.absolutePath());

    // If the free space is unknown, let the conversion try.
    if (!storage.isValid())
        return true;

    return storage.bytesAvailable() >= info.size() + kSpaceMarginBytes;
}

bool replaceFile(const QString& source, const QString& destination)
{
    std::error_code error;

    std::filesystem::rename(std::filesystem::path(source.toStdU16String()),
                            std::filesystem::path(destination.toStdU16String()), error);

    return !error;
}

} // namespace

bool remuxToStandardMp4(const QString& path)
{
    const QFileInfo info(path);

    if (!info.exists() || info.size() == 0)
        return false;

    if (!hasEnoughSpace(info))
    {
        qWarning() << "Not enough free space to convert" << path;
        return false;
    }

    const QString tempPath =
        info.absolutePath() + "/" + info.completeBaseName() + ".converting.tmp";

    const QByteArray inputName = path.toUtf8();
    const QByteArray outputName = tempPath.toUtf8();

    AVFormatContext* input = nullptr;
    AVFormatContext* output = nullptr;
    AVPacket* packet = av_packet_alloc();

    bool success = false;

    do
    {
        if (!packet)
            break;

        if (avformat_open_input(&input, inputName.constData(), nullptr, nullptr) < 0)
            break;

        if (avformat_find_stream_info(input, nullptr) < 0)
            break;

        if (avformat_alloc_output_context2(&output, nullptr, "mp4", outputName.constData()) < 0)
            break;

        bool streamsOk = true;

        for (unsigned i = 0; i < input->nb_streams; ++i)
        {
            AVStream* outStream = avformat_new_stream(output, nullptr);

            if (!outStream ||
                avcodec_parameters_copy(outStream->codecpar, input->streams[i]->codecpar) < 0)
            {
                streamsOk = false;
                break;
            }

            outStream->codecpar->codec_tag = 0;
            outStream->time_base = input->streams[i]->time_base;
        }

        if (!streamsOk)
            break;

        if (avio_open(&output->pb, outputName.constData(), AVIO_FLAG_WRITE) < 0)
            break;

        // No "movflags": the muxer writes a standard MP4 (index at the end).
        if (avformat_write_header(output, nullptr) < 0)
            break;

        bool copyOk = true;
        int ret = 0;

        while ((ret = av_read_frame(input, packet)) >= 0)
        {
            AVStream* inStream = input->streams[packet->stream_index];
            AVStream* outStream = output->streams[packet->stream_index];

            av_packet_rescale_ts(packet, inStream->time_base, outStream->time_base);
            packet->pos = -1;

            const int writeRet = av_interleaved_write_frame(output, packet);
            av_packet_unref(packet);

            if (writeRet < 0)
            {
                copyOk = false;
                break;
            }
        }

        if (!copyOk || ret != AVERROR_EOF)
            break;

        if (av_write_trailer(output) < 0)
            break;

        success = true;
    } while (false);

    av_packet_free(&packet);

    if (output)
    {
        if (output->pb)
            avio_closep(&output->pb);

        avformat_free_context(output);
    }

    if (input)
        avformat_close_input(&input);

    // The original is replaced only when the new file is complete.
    if (success)
        success = replaceFile(tempPath, path);

    if (!success)
        QFile::remove(tempPath);

    return success;
}

} // namespace fluvel::ffmpeg_utils
