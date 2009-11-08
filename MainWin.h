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
#include <stdlib.h>

#include "Library.h"


const int32 UPDATE_PROGRESS = 'UPPR';


class MainWin : public BWindow {
    // GUI elements
    BButton *start;
    BButton *next;
    BButton *stop;
    BStatusBar *progress;

    Library *library;

    // Playback state
    int position;
    bool shuffle;

    void MainView();
    void LoadLibrary(BDirectory *dir);

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
