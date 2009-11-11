#include "Main.h"
#include "MainWin.h"
#include "Messages.h"
#include "LibraryItem.h"

#include <Entry.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>

#define be_app ((MusicPlayer *)be_app)


bigtime_t MainWin::time;
BMediaTrack *MainWin::playTrack;
BMediaFile *MainWin::mediaFile;
BSoundPlayer *MainWin::sp;
media_format MainWin::playFormat;

MainWin::MainWin() :
    BWindow(BRect(0, 0, 1, 1), "Music Player", B_TITLED_WINDOW,
            B_ASYNCHRONOUS_CONTROLS | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS),
    position(1736),
    shuffle(true) {
    MainView();
}

bool
MainWin::QuitRequested() {
    be_app_messenger.SendMessage(B_QUIT_REQUESTED);
    return BWindow::QuitRequested();
}

void
MainWin::MessageReceived(BMessage *message) {
    switch(message->what) {
        case MSG_PLAY:
            play->Hide();
            pause->Show();
            Play();
            break;
        case MSG_PAUSE:
            pause->Hide();
            play->Show();
            Pause();
            break;
        case MSG_STOP:
            Stop();
            break;
        case MSG_NEXT:
            Next();
            break;
        case MSG_TOGGLE_SHUFFLE:
            shuffle = !shuffle;
            shuffleTick->SetValue(shuffle);
            break;
        case MSG_UPDATE_PROGRESS: {
            float current;
            const char *now;

            message->FindFloat("current", &current);
            message->FindString("time", &now);

            progress->Update(current - progress->CurrentValue(), now);
            break;
        }
        default:
            BWindow::MessageReceived(message);
    }
}

void
MainWin::PlayBuffer(void *cookie, void *buffer, size_t size, const media_raw_audio_format &format) {
    int64 frames = 0;

    playTrack->ReadFrames(buffer, &frames);

    MainWin *window = (MainWin *)cookie;

    bigtime_t now;

    if (sp->CurrentTime() < time)
        now = time + sp->CurrentTime();
    else
        now = time = sp->CurrentTime();

    char timestamp[100];

    MainWin::Timestamp(timestamp, time);

    BMessage *update = new BMessage(MSG_UPDATE_PROGRESS);
    update->AddFloat("current", (float) now);
    update->AddString("time", timestamp);

    window->PostMessage(update);

    if (frames <= 0) {
        sp->SetHasData(false);
        window->PostMessage(MSG_NEXT);
    }
}

void
MainWin::Notifier(void *cookie, BSoundPlayer::sound_player_notification what, ...) {
    printf("Got sound player notification. %d\n", what == BSoundPlayer::B_SOUND_DONE);
}

void
MainWin::Timestamp(char *target, bigtime_t length) {
    int seconds = (float) length / 1000000.0;
    int minutes = (float) seconds / 60.0;
    seconds -= (60 * minutes);

    sprintf(target, "%d:%02d", minutes, seconds);
}

void
MainWin::MainView() {
    play = new BButton(
        BRect(0, 0, 70, 0),
        "playButton",
        "Play",
        new BMessage(MSG_PLAY)
    );

    pause = new BButton(
        BRect(0, 0, 70, 0),
        "pauseButton",
        "Pause",
        new BMessage(MSG_PAUSE)
    );
    pause->Hide();

    next = new BButton(
        BRect(0, 0, 70, 0),
        "nextButton",
        "Next",
        new BMessage(MSG_NEXT)
    );

    stop = new BButton(
        BRect(0, 0, 70, 0),
        "stopButton",
        "Stop",
        new BMessage(MSG_STOP)
    );

    shuffleTick = new BCheckBox(
        BRect(0, 0, 80, 0),
        "shuffleTick",
        "Shuffle",
        new BMessage(MSG_TOGGLE_SHUFFLE)
    );
    shuffleTick->SetValue(shuffle);

    progress = new BStatusBar(
        BRect(0, 0, 100, 0),
        "playStatus"
    );

    SetLayout(new BGroupLayout(B_HORIZONTAL));

    AddChild(
        BGroupLayoutBuilder(B_VERTICAL, 5)
        .Add(BGroupLayoutBuilder(B_HORIZONTAL, 5)
             .Add(play)
             .Add(pause)
             .AddGlue()
             .Add(next)
             .AddGlue()
             .Add(stop)
             .AddGlue()
             .Add(shuffleTick))
        .AddGlue()
        .Add(progress)
        .SetInsets(10, 10, 10, 10)
    );

    CenterOnScreen();
}


void
MainWin::Play() {
    if (sp && sp->HasData()) {
        sp->Start();
        return;
    }

    printf("Starting at position %d.\n", position);

    status_t err;

    LibraryItem *item = dynamic_cast<LibraryItem *>(be_app->library->list->ItemAt(position));
    printf("Playing track %s.\n", item->entry.name);

    mediaFile = new BMediaFile(&item->entry);
    if ((err = mediaFile->InitCheck()) != B_OK) {
        printf("Play: Initiating media file failed: %s; skipping.\n", strerror(err));
        delete mediaFile;
        Next();
        return;
    }

    playTrack = 0;
    for (int i = 0; i < mediaFile->CountTracks(); i++) {
        playTrack = mediaFile->TrackAt(i);
        playFormat.type = B_MEDIA_RAW_AUDIO;

        if (playTrack->DecodedFormat(&playFormat) == B_OK)
            break;

        if (playTrack)
            mediaFile->ReleaseTrack(playTrack);
    }

    if (playTrack) {
        sp = new BSoundPlayer(&playFormat.u.raw_audio, "playfile", PlayBuffer, Notifier);
        sp->SetCookie(this);
        sp->SetVolume(1.0f);
        sp->SetHasData(true);
        sp->Start();

        char length[100];
        Timestamp(length, playTrack->Duration());

        progress->Reset(NULL, length);
        progress->SetMaxValue(playTrack->Duration());

        BMessage *select = new BMessage(MSG_LIBRARY_SELECT);
        select->AddUInt32("position", position);

        be_app->library->PostMessage(select);

        printf("Playback started.\n");
    } else {
        printf("ERROR: No valid track found. Skipping.\n");
        Next();
    }
}


void
MainWin::Pause() {
    printf("Pausing playback.");

    sp->Stop();
}

void
MainWin::Stop() {
    printf("Stopping playback.\n");

    sp->Stop();
    delete sp;
}


void
MainWin::Next() {
    printf("Next track.\n");

    if (shuffle)
        position = rand() % be_app->library->list->CountItems();
    else
        position += 1;

    if (sp) {
        sp->Stop();
        sp = 0;
        time = 0;
    }

    Play();
}
