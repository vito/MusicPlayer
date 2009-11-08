#include "Playlist.h"

void
Playlist::Add(BEntry file) {
    PlaylistFile *entry = new PlaylistFile;

    entry->file = file;
    entry->next = head;

    head = entry;

    size++;
}

void
Playlist::Pop() {
    head = head->next;
    size--;
}

int
Playlist::Count() {
    return size;
}

BEntry
Playlist::At(int offset) {
    PlaylistFile cur = *head;

    for (int i = 1; i < (Count() - offset); i++)
        cur = *cur.next;

    return cur.file;
}

