#include "LibraryWin.h"
#include "LibraryItem.h"
#include "Messages.h"

#include <Application.h>
#include <Entry.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <NodeMonitor.h>
#include <String.h> // strerror

const uint32 MSG_SELECT ='SLCT';


LibraryWin::LibraryWin() :
    BWindow(BRect(100, 100, 500, 300), "Library", B_TITLED_WINDOW,
            B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS) {
    node_ref nref;
    status_t err;

    LibraryView();
    Show();

    BDirectory *dir = new BDirectory("/Book/Music");
    if (dir->InitCheck() == B_OK) {
        dir->GetNodeRef(&nref);
        printf("Directory node ref received.\n");
        err = watch_node(&nref, B_WATCH_DIRECTORY, be_app_messenger);

        if (err != B_OK)
            printf("Directory node could not be watched.\n");
        else
            printf("Watching directory node.\n");

        LoadLibrary(dir);

        printf("Library loaded; entries: %d\n", (int) list->CountItems());
    } else
        printf("Library could not be loaded.");
}

bool
LibraryWin::QuitRequested() {
    return BWindow::QuitRequested();
}

void
LibraryWin::MessageReceived(BMessage *message) {
    switch(message->what) {
        case MSG_LIBRARY_SELECT:
            uint32 position;
            message->FindUInt32("position", &position);

            list->Select(position);
            list->ScrollToSelection();
            break;
        case MSG_LIBRARY_ADD:
            dev_t device;
            ino_t dir;
            const char *name;

            message->FindInt32("device", &device);
            message->FindInt64("dir", &dir);
            message->FindString("name", &name);

            list->AddItem(new LibraryItem(entry_ref(device, dir, name)));

            break;
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
        default:
            BWindow::MessageReceived(message);
    }
}

void
LibraryWin::LibraryView() {
    list = new BListView(
        BRect(0, 0, 400, 300),
        "libraryList"
    );

    scroll = new BScrollView(
        "scrollLibrary",
        list,
        0,
        B_SUPPORTS_LAYOUT,
        false,
        true
    );

    SetLayout(new BGroupLayout(B_HORIZONTAL));

    AddChild(
        BGroupLayoutBuilder(B_VERTICAL, 5)
        .Add(scroll)
        .SetInsets(10, 10, 10, 10)
    );
}

void
LibraryWin::LoadLibrary(BDirectory *dir) {
    status_t err;

    BEntry entry;
    entry_ref ref;

    while ((err = dir->GetNextEntry(&entry)) == B_OK) {
        entry.GetRef(&ref);

        if (entry.IsDirectory()) {
            BDirectory *entryDir = new BDirectory(&entry);
            LoadLibrary(entryDir);
            delete entryDir;
        } else {
            BMessage *add = new BMessage(MSG_LIBRARY_ADD);
            add->AddInt32("device", ref.device);
            add->AddInt64("dir", ref.directory);
            add->AddString("name", ref.name);

            PostMessage(add);
        }
    }

    if (err != B_ENTRY_NOT_FOUND)
        printf("LoadLibrary: Error encountered at %d added: %s.\n", (int) list->CountItems(), strerror(err));
}
