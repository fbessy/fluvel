// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "snapshot_worker.hpp"

#include "application_settings.hpp"
#include "file_utils.hpp"

namespace fluvel
{

SnapshotWorker::SnapshotWorker()
{
    workerThread_ = std::thread(&SnapshotWorker::processQueue, this);
}

SnapshotWorker::~SnapshotWorker()
{
    {
        QMutexLocker locker(&mutex_);
        stopping_ = true;
    }

    condition_.wakeOne();

    if (workerThread_.joinable())
        workerThread_.join();
}

bool SnapshotWorker::enqueue(const QImage& image)
{
    {
        QMutexLocker locker(&mutex_);

        if (stopping_)
            return false;

        if (imageQueue_.size() >= kMaxPendingSnapshots)
            return false;

        imageQueue_.push_back(image);
    }

    condition_.wakeOne();

    return true;
}

void SnapshotWorker::processQueue()
{
    for (;;)
    {
        QImage image;

        {
            QMutexLocker locker(&mutex_);

            while (imageQueue_.empty() && !stopping_)
                condition_.wait(&mutex_);

            if (imageQueue_.empty() && stopping_)
                break;

            image = std::move(imageQueue_.front());
            imageQueue_.pop_front();
        }

        saveImage(image);
    }
}

void SnapshotWorker::saveImage(const QImage& image)
{
    const auto& preferences = ApplicationSettings::instance().snapshotPreferences();

    const QString fileName = file_utils::buildOutputFileName(
        preferences.directory, preferences.baseName,
        QString::fromLatin1(preferences.preferredFormat), preferences.appendTimestamp);

    if (!image.save(fileName, preferences.preferredFormat.constData()))
    {
        emit snapshotError(tr("Failed to save snapshot: %1").arg(fileName));
        return;
    }

    emit snapshotSaved(fileName);
}

} // namespace fluvel