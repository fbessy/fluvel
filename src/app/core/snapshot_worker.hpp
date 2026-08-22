// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include <QImage>
#include <QMutex>
#include <QObject>
#include <QWaitCondition>

#include <cstddef>
#include <deque>
#include <thread>

namespace fluvel
{

/**
 * @brief Asynchronously saves captured images to disk.
 *
 * SnapshotWorker owns a dedicated worker thread and a FIFO queue of images.
 * Snapshot requests are queued and processed sequentially so that image
 * encoding and file I/O do not block the caller thread.
 *
 * The queue has a bounded capacity to prevent an excessive number of
 * pending snapshots from accumulating in memory.
 */
class SnapshotWorker : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Maximum number of snapshots waiting to be saved.
     */
    static constexpr std::size_t kMaxPendingSnapshots{8};

    /**
     * @brief Constructs and starts the snapshot worker.
     */
    SnapshotWorker();

    /**
     * @brief Stops the worker and waits for the worker thread to finish.
     */
    ~SnapshotWorker() override;

    SnapshotWorker(const SnapshotWorker&) = delete;
    SnapshotWorker& operator=(const SnapshotWorker&) = delete;

    /**
     * @brief Queues an image for asynchronous saving.
     *
     * @param image Image to save.
     * @return true if the image was queued, false if the queue is full.
     */
    bool enqueue(const QImage& image);

signals:
    /**
     * @brief Emitted when a snapshot has been successfully saved.
     *
     * @param fileName Path of the saved snapshot.
     */
    void snapshotSaved(const QString& fileName);

    /**
     * @brief Emitted when a snapshot cannot be saved.
     *
     * @param message Error description.
     */
    void snapshotError(const QString& message);

private:
    /**
     * @brief Processes queued snapshots in the worker thread.
     */
    void processQueue();

    /**
     * @brief Saves one image according to the current snapshot preferences.
     *
     * @param image Image to save.
     */
    void saveImage(const QImage& image);

    std::thread workerThread_;

    QMutex mutex_;
    QWaitCondition condition_;

    std::deque<QImage> imageQueue_;

    bool stopping_{false};
};

} // namespace fluvel