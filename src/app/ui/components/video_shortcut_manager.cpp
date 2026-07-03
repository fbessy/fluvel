// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include "video_shortcut_manager.hpp"

#include <QKeySequence>
#include <QShortcut>
#include <QWidget>

namespace fluvel
{

VideoShortcutManager::VideoShortcutManager(QWidget* parent)
    : QObject(parent)
{
    createShortcuts(parent);
}

void VideoShortcutManager::createShortcuts(QWidget* parent)
{
    auto makeShortcut = [parent](QKeySequence key)
    {
        auto* shortcut = new QShortcut(key, parent);
        shortcut->setContext(Qt::WindowShortcut);
        return shortcut;
    };

    playPauseShortcut_ = makeShortcut(Qt::Key_Space);
    seekBackwardShortcut_ = makeShortcut(Qt::Key_Left);
    seekForwardShortcut_ = makeShortcut(Qt::Key_Right);
    volumeUpShortcut_ = makeShortcut(Qt::Key_Up);
    volumeDownShortcut_ = makeShortcut(Qt::Key_Down);
    muteShortcut_ = makeShortcut(Qt::Key_M);
    fullscreenShortcut_ = makeShortcut(Qt::Key_F);
    escapeShortcut_ = makeShortcut(Qt::Key_Escape);

    connect(playPauseShortcut_, &QShortcut::activated, this,
            &VideoShortcutManager::playPauseRequested);

    connect(seekForwardShortcut_, &QShortcut::activated, this,
            [this]()
            {
                emit seekRequested(+5000);
            });

    connect(seekBackwardShortcut_, &QShortcut::activated, this,
            [this]()
            {
                emit seekRequested(-5000);
            });

    connect(volumeUpShortcut_, &QShortcut::activated, this,
            [this]()
            {
                emit volumeRequested(+5);
            });

    connect(volumeDownShortcut_, &QShortcut::activated, this,
            [this]()
            {
                emit volumeRequested(-5);
            });

    connect(muteShortcut_, &QShortcut::activated, this, &VideoShortcutManager::toggleMuteRequested);

    connect(fullscreenShortcut_, &QShortcut::activated, this,
            &VideoShortcutManager::toggleFullscreenRequested);

    connect(escapeShortcut_, &QShortcut::activated, this, &VideoShortcutManager::escapeRequested);
}

void VideoShortcutManager::setEnabled(bool enabled)
{
    const QList<QShortcut*> shortcuts = {
        playPauseShortcut_,  seekForwardShortcut_, seekBackwardShortcut_, volumeUpShortcut_,
        volumeDownShortcut_, muteShortcut_,        fullscreenShortcut_,   escapeShortcut_};

    for (QShortcut* shortcut : shortcuts)
    {
        if (shortcut)
            shortcut->setEnabled(enabled);
    }
}

} // namespace fluvel