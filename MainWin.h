#ifndef __MAIN_WIN_H
#define __MAIN_WIN_H

#include <Button.h>
#include <Directory.h>
#include <MediaFile.h>
#include <MediaTrack.h>
#include <SoundPlayer.h>
#include <StatusBar.h>
#include <Window.h>

#include <stdio.h>

#include "Library.h"


const int32 UPDATE_PROGRESS = 'UPPR';


class MainWin : public BWindow {
    // GUI elements
    BButton *start;
    BButton *next;
    BButton *stop;
    BStatusBar *progress;

    // Playback state
    int position;
    Library *library;

    void MainView();
    void LoadLibrary(BDirectory *dir);
    BSoundPlayer *Player(media_multi_audio_format*, char*, void (*)(void*, void*, size_t, const media_raw_audio_format&));

    void OnStart();
    void OnStop();
    void OnNext();

public:
    static BMediaTrack *playTrack;
    static BMediaFile *mediaFile;
    static BSoundPlayer *sp;
    static media_format playFormat;

    static void PlayBuffer(void *cookie, void *buffer, size_t size, const media_raw_audio_format &format);
    static void Notifier(void *cookie, BSoundPlayer::sound_player_notification what, ...);

    MainWin();

    bool QuitRequested();

    void MessageReceived(BMessage *message);
};

#endif
