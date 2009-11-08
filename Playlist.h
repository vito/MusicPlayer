#ifndef __PLAYLIST_H
#define __PLAYLIST_H

#include <Entry.h>


struct PlaylistFile {
    BEntry file;
    PlaylistFile *next;
};


class Playlist {
    PlaylistFile *head;
    int size;

public:
    Playlist() {}

    BEntry At(int offset);

    void Add(BEntry file);

    void Pop();

    int Count();
};

#endif
