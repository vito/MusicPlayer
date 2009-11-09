#include "Main.h"
#include "MainWin.h"
#include "LibraryItem.h"

#include <Entry.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>

#define be_app ((MusicPlayer *)be_app)


const uint32 MSG_SELECT = 'SLCT';


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
        case UPDATE_PROGRESS: {
            float current;
            const char *now;

            message->FindFloat("current", &current);
            message->FindString("time", &now);

            progress->Update(current - progress->CurrentValue(), now);
            break;
        }
        case START:
            OnStart();
            break;
        case STOP:
            OnStop();
            break;
        case NEXT:
            OnNext();
            break;
        case SHUFFLE:
            shuffle = !shuffle;
            shuffleTick->SetValue(shuffle);
            break;
        default:
            BWindow::MessageReceived(message);
    }
}

void
MainWin::PlayBuffer(void *cookie, void *buffer, size_t size, const media_raw_audio_format &format) {
    MainWin *window = (MainWin *)cookie;
    int64 frames = 0;

    playTrack->ReadFrames(buffer, &frames);

    char now[100];

    MainWin::Timestamp(now, sp->CurrentTime());

    BMessage *update = new BMessage(UPDATE_PROGRESS);
    update->AddFloat("current", (float) sp->CurrentTime());
    update->AddString("time", now);

    window->PostMessage(update);

    if (frames <= 0) {
        sp->SetHasData(false);
        window->PostMessage(NEXT);
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
    start = new BButton(
        BRect(0, 0, 80, 0),
        "startButton",
        "Start",
        new BMessage(START)
    );

    next = new BButton(
        BRect(0, 0, 80, 0),
        "nextButton",
        "Next",
        new BMessage(NEXT)
    );

    stop = new BButton(
        BRect(0, 0, 80, 0),
        "stopButton",
        "Stop",
        new BMessage(STOP)
    );

    shuffleTick = new BCheckBox(
        BRect(0, 0, 80, 0),
        "shuffleTick",
        "Shuffle",
        new BMessage(SHUFFLE)
    );
    shuffleTick->SetValue(shuffle);

    progress = new BStatusBar(
        BRect(0, 0, 100, 0),
        "playStatus"
    );

    SetLayout(new BGroupLayout(B_HORIZONTAL));

    AddChild(
        BGroupLayoutBuilder(B_VERTICAL, 5)
        .Add(BGroupLayoutBuilder(B_HORIZONTAL, 10)
             .Add(start)
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
MainWin::OnStart() {
    printf("Starting at position %d.\n", position);

    status_t err;

    LibraryItem *item = dynamic_cast<LibraryItem *>(be_app->library->list->ItemAt(position));
    printf("Playing track %s.\n", item->entry.name);

    mediaFile = new BMediaFile(&item->entry);
    if ((err = mediaFile->InitCheck()) != B_OK) {
        printf("OnStart: Initiating media file failed: %s; skipping.\n", strerror(err));
        delete mediaFile;
        OnNext();
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

        BMessage *select = new BMessage(MSG_SELECT);
        select->AddUInt32("position", position);

        be_app->library->PostMessage(select);

        printf("Playback started.\n");
    } else {
        printf("ERROR: No valid track found. Skipping.\n");
        OnNext();
    }
}


void
MainWin::OnStop() {
    printf("Stopping playback.\n");

    sp->Stop();
}


void
MainWin::OnNext() {
    printf("Next track.\n");

    if (shuffle)
        position = rand() % be_app->library->list->CountItems();
    else
        position += 1;

    if (sp)
        sp->Stop();

    OnStart();
}
