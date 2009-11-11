#ifndef __MAIN_WIN_H
#define __MAIN_WIN_H

#include <Button.h>
#include <CheckBox.h>
#include <MediaFile.h>
#include <MediaTrack.h>
#include <SoundPlayer.h>
#include <StatusBar.h>
#include <Window.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


class MainWin : public BWindow {
    // GUI elements
    BButton *play;
    BButton *pause;
    BButton *next;
    BButton *stop;
    BCheckBox *shuffleTick;
    BStatusBar *progress;

    // Playback state
    int32 position;
    bool shuffle;

    void MainView();

    void Play();
    void Pause();
    void Stop();
    void Next();

public:
    static bigtime_t time;
    static BMediaTrack *playTrack;
    static BMediaFile *mediaFile;
    static BSoundPlayer *sp;
    static media_format playFormat;

    static void PlayBuffer(void *cookie, void *buffer, size_t size, const media_raw_audio_format &format);
    static void Notifier(void *cookie, BSoundPlayer::sound_player_notification what, ...);

    static void Timestamp(char *target, bigtime_t length);

    MainWin();

    bool QuitRequested();

    void MessageReceived(BMessage *message);
};

#endif
