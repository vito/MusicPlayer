#include "LibraryWin.h"
#include "LibraryItem.h"

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
        case MSG_SELECT:
            uint32 position;
            message->FindUInt32("position", &position);

            list->Select(position);
            list->ScrollToSelection();
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

        printf("Loading... %s\n", ref.name);

        if (entry.IsDirectory()) {
            BDirectory *entryDir = new BDirectory(&entry);
            LoadLibrary(entryDir);
            delete entryDir;
        } else {
            printf("Adding item.\n");
            list->AddItem(new LibraryItem(ref));
            printf("Added.\n");
        }
    }

    if (err != B_ENTRY_NOT_FOUND)
        printf("LoadLibrary: Error encountered at %d added: %s.\n", (int) list->CountItems(), strerror(err));
}
