#include <Alert.h>
#include <Application.h>
#include <Directory.h>
#include <Entry.h>
#include <MediaFile.h>
#include <MediaTrack.h>
#include <NodeMonitor.h>
#include <Roster.h>
#include <SoundPlayer.h>

#include <stdio.h>

#include "MainWin.h"
#include "Playlist.h"


const int32 START = 'STRT';
const int32 STOP = 'STOP';
const int32 NEXT = 'NEXT';


class MusicPlayer : public BApplication {
    MainWin *win;
    Playlist *playlist;
    int position;

    bool fMediaServerRunning;
    bool fMediaAddOnServerRunning;

    BMediaFile *mediaFile;
    BMediaTrack *playTrack;
    BSoundPlayer *sp;
    media_format playFormat;

    void PlayBuffer(void *cookie, void *buffer, size_t size, const media_raw_audio_format &format) {
        int64 frames = 0;

        playTrack->ReadFrames(buffer, &frames);

        if (frames <= 0) {
            sp->SetHasData(false);
        }
    }

public:
    MusicPlayer() :
        BApplication("application/music-player"),
        position(43),
        mediaFile(NULL),
        playTrack(NULL),
        sp(0) {}

    void ReadyToRun() {
        node_ref nref;
        status_t err;

        win = new MainWin();
        win->Show();

        BDirectory *dir = new BDirectory("/Book/Music");
        if (dir->InitCheck() == B_OK) {
            dir->GetNodeRef(&nref);
            printf("Directory node ref received.\n");
            err = watch_node(&nref, B_WATCH_DIRECTORY, be_app_messenger);

            if (err != B_OK)
                printf("Directory node could not be watched.\n");
            else
                printf("Watching directory node.\n");

            playlist = new Playlist();
            LoadPlaylist(dir);

            printf("Playlist loaded; entries: %d\n", playlist->Count());
        } else
            printf("Playlist could not be loaded.");
    }

    void MessageReceived(BMessage *msg) {
        switch (msg->what) {
            case B_NODE_MONITOR:
                printf("Main app got node monitor message.\n");

                int32 opcode;
                msg->FindInt32("opcode", &opcode);
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
            case STOP:
                printf("Playback stopping. %.1f%%\n", ((float) sp->CurrentTime() / (float) playTrack->Duration()) * 100);

                //win->mainView->SendMessage((const uint32) 'TODO', 0.0);
                sp->Stop();
                break;
            case NEXT:
                position += 1;
                printf("Next in main.\n");

                if (sp) {
                    sp->Stop();
                    printf("Stopped.\n");
                }
            case START: {
                printf("Starting at position %d.\n", position);
                entry_ref file;
                status_t err;

                if (sp) {
                    delete sp;
                    delete mediaFile;
                }

                char name[B_FILE_NAME_LENGTH];

                BEntry entry = playlist->At(position);
                printf("Entry received. %d\n", entry.InitCheck() == B_OK);

                entry.GetName(name);
                printf("Playing file %s.\n", name);

                if ((err = entry.GetRef(&file)) != B_OK) {
                    printf("Ref failed. %s\n", strerror(err));
                    BAlert *alert = new BAlert("notfoundAlert", "File not found.", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
                    alert->Go();
                    break;
                }

                BMediaFile *mediaFile = new BMediaFile(&file);

                if ((err = mediaFile->InitCheck()) != B_OK) {
                    printf("Init failed. %s\n", strerror(err));
                    BAlert *alert = new BAlert("invalidAlert", "File invalid.", "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
                    alert->Go();
                    break;
                }

                for (int i = 0; i < mediaFile->CountTracks(); i++) {
                    BMediaTrack *track = mediaFile->TrackAt(i);
                    playFormat.type = B_MEDIA_RAW_AUDIO;

                    if (track->DecodedFormat(&playFormat) == B_OK) {
                        playTrack = track;
                        break;
                    }

                    if (track)
                        mediaFile->ReleaseTrack(track);
                }

                //sp = new BSoundPlayer(&playFormat.u.raw_audio, "playfile", &PlayBuffer, NULL, NULL);
                //sp->SetVolume(1.0f);
                //sp->SetHasData(true);
                //sp->Start();

                printf("Playback started.\n");

                break;
            }
            default:
                printf("Main app got message.\n");
        }
    }

protected:
    void LoadPlaylist(BDirectory *dir) {
        status_t err;

        BEntry entry;
        char name[B_FILE_NAME_LENGTH];

        while ((err = dir->GetNextEntry(&entry)) == B_OK) {
            entry.GetName(name);

            if (entry.IsDirectory()) {
                BDirectory *entryDir = new BDirectory(&entry);
                LoadPlaylist(entryDir);
                delete entryDir;
            } else
                playlist->Add(entry);
        }

        if (err != B_ENTRY_NOT_FOUND)
            printf("ERROR at %d added: %s.\n", playlist->Count(), strerror(err));
    }
};


int main(void) {
    MusicPlayer app;
    app.Run();
    return 0;
}
