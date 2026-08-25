// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "camera_video_source.hpp"
#include "camera_format_utils.hpp"

#include <QMediaDevices>
#include <QVideoSink>

#include <algorithm>

namespace fluvel
{

CameraVideoSource::CameraVideoSource(QObject* parent)
    : QObject(parent)
{
}

CameraVideoSource::~CameraVideoSource()
{
    stop();
}

bool CameraVideoSource::start(const CameraConfig& config)
{
    if (camera_)
        return false;

    const auto cameras = QMediaDevices::videoInputs();

    for (const auto& device : cameras)
    {
        if (device.id() != config.deviceId)
            continue;

        camera_ = new QCamera(device, this);

        if (!config.deviceFormat.isNull())
        {
            const auto formats = device.videoFormats();

            const auto it = std::find_if(formats.begin(), formats.end(),
                                         [&](const QCameraFormat& format)
                                         {
                                             return camera_utils::isSameCameraFormat(
                                                 format, config.deviceFormat);
                                         });

            if (it != formats.end())
                camera_->setCameraFormat(*it);
        }

        captureSession_.setCamera(camera_);

        connect(camera_, &QCamera::errorOccurred, this, &CameraVideoSource::error);

        camera_->start();

        return true;
    }

    return false;
}

void CameraVideoSource::stop()
{
    if (!camera_)
        return;

    disconnect(camera_, &QCamera::errorOccurred, this, &CameraVideoSource::error);

    camera_->stop();

    captureSession_.setCamera(nullptr);

    delete camera_;
    camera_ = nullptr;
}

void CameraVideoSource::setVideoSink(QVideoSink* sink)
{
    captureSession_.setVideoSink(sink);
}

CameraInfo CameraVideoSource::cameraInfo() const
{
    CameraInfo info;

    if (!camera_)
        return info;

    info.deviceId = camera_->cameraDevice().id();
    info.deviceFormat = camera_->cameraFormat();
    info.description = camera_->cameraDevice().description();

    return info;
}

bool CameraVideoSource::isActive() const noexcept
{
    return camera_ != nullptr;
}

} // namespace fluvel