#include <Alert.h>
#include <Application.h>
#include <Entry.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <NodeMonitor.h>

#include "MainWin.h"

const int32 START = 'STRT';
const int32 STOP = 'STOP';
const int32 NEXT = 'NEXT';

BMediaTrack *MainWin::playTrack;
BMediaFile *MainWin::mediaFile;
BSoundPlayer *MainWin::sp;
media_format MainWin::playFormat;

void
MainWin::PlayBuffer(void *cookie, void *buffer, size_t size, const media_raw_audio_format &format) {
    int64 frames = 0;

    playTrack->ReadFrames(buffer, &frames);

    if (frames <= 0) {
        printf("Finished playing.\n");

        MainWin *window = (MainWin *)cookie;

        printf("Stopping sound buffer.\n");
        sp->SetHasData(false);

        printf("Sending NEXT to window.\n");

        window->PostMessage(NEXT);
    }
}

void
MainWin::Notifier(void *cookie, BSoundPlayer::sound_player_notification what, ...) {
    printf("Got sound player notification. %d\n", what == BSoundPlayer::B_SOUND_DONE);
}

MainWin::MainWin() :
    BWindow(BRect(0, 0, 1, 1), "Music Player", B_TITLED_WINDOW,
            B_ASYNCHRONOUS_CONTROLS | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS),
    position(52) {
    node_ref nref;
    status_t err;

    MainView();

    BDirectory *dir = new BDirectory("/Book/Music");
    if (dir->InitCheck() == B_OK) {
        dir->GetNodeRef(&nref);
        printf("Directory node ref received.\n");
        err = watch_node(&nref, B_WATCH_DIRECTORY, be_app_messenger);

        if (err != B_OK)
            printf("Directory node could not be watched.\n");
        else
            printf("Watching directory node.\n");

        library = new Library();
        LoadLibrary(dir);

        printf("Library loaded; entries: %d\n", library->Count());
    } else
        printf("Library could not be loaded.");
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
            float percentage;
            const char *now;
            const char *total;

            message->FindFloat("percentage", &percentage);
            message->FindString("now", &now);
            message->FindString("total", &total);

            progress->Reset();
            progress->Update(percentage, now, total);
            break;
        }
        case B_NODE_MONITOR: {
            printf("Main app got node monitor message.\n");

            int32 opcode;
            message->FindInt32("opcode", &opcode);
            switch (opcode) {
                case B_ENTRY_CREATED:
                    printf("Entry created.\n");
                    break;
                case B_ENTRY_REMOVED:
                    printf("Entry removed.\n");
                    break;
                default:
                    printf("Opcode: %d\n", (int) opcode);
            }
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
        default:
            BWindow::MessageReceived(message);
    }
}

void
MainWin::LoadLibrary(BDirectory *dir) {
    status_t err;

    BEntry entry;
    char name[B_FILE_NAME_LENGTH];

    while ((err = dir->GetNextEntry(&entry)) == B_OK) {
        entry.GetName(name);

        if (entry.IsDirectory()) {
            BDirectory *entryDir = new BDirectory(&entry);
            LoadLibrary(entryDir);
            delete entryDir;
        } else
            library->Add(entry);
    }

    if (err != B_ENTRY_NOT_FOUND)
        printf("LoadLibrary: Error encountered at %d added: %s.\n", library->Count(), strerror(err));
}

void
MainWin::MainView() {
    //SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

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

    progress = new BStatusBar(
        BRect(0, 0, 100, 0),
        "playStatus",
        "0:00",
        "5:00"
    );

    SetLayout(new BGroupLayout(B_HORIZONTAL));

    AddChild(
        BGroupLayoutBuilder(B_VERTICAL, 5)
        .Add(BGroupLayoutBuilder(B_HORIZONTAL, 10)
             .Add(start)
             .AddGlue()
             .Add(next)
             .AddGlue()
             .Add(stop))
        .AddGlue()
        .Add(progress)
        .SetInsets(10, 10, 10, 10)
    );

    CenterOnScreen();
}


void
MainWin::OnStart() {
    printf("Starting at position %d.\n", position);

    entry_ref file;
    status_t err;

    char name[B_FILE_NAME_LENGTH];

    delete mediaFile;

    BEntry entry = library->At(position);
    printf("Entry received. Ok?: %d\n", entry.InitCheck() == B_OK);

    entry.GetName(name);
    printf("Playing track %s.\n", name);

    if ((err = entry.GetRef(&file)) != B_OK) {
        printf("OnStart: Getting reference for entry failed: %s\n", strerror(err));
        BAlert *alert = new BAlert("notfoundAlert", "File not found.", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
        alert->Go();
        return;
    }

    mediaFile = new BMediaFile(&file);
    if ((err = mediaFile->InitCheck()) != B_OK) {
        printf("OnStart: Initiating media file failed: %s\n", strerror(err));
        BAlert *alert = new BAlert("invalidAlert", "File invalid.", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
        alert->Go();
        delete mediaFile;
        return;
    }

    for (int i = 0; i < mediaFile->CountTracks(); i++) {
        playTrack = mediaFile->TrackAt(i);
        playFormat.type = B_MEDIA_RAW_AUDIO;

        if (playTrack->DecodedFormat(&playFormat) == B_OK)
            break;

        if (playTrack)
            mediaFile->ReleaseTrack(playTrack);
    }

    MainWin::sp = new BSoundPlayer(&playFormat.u.raw_audio, "playfile", PlayBuffer, Notifier);
    sp->SetCookie(this);
    sp->SetVolume(1.0f);
    sp->SetHasData(true);
    sp->Start();

    printf("Playback started.\n");
}


void
MainWin::OnStop() {
    printf("Stopping playback.\n");

    MainWin::sp->Stop();
}


void
MainWin::OnNext() {
    printf("Next track.\n");

    position += 1;

    if (sp)
        sp->Stop();

    OnStart();
}
