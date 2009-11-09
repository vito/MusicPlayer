#ifndef __LIBRARY_WIN_H
#define __LIBRARY_WIN_H

#include <Directory.h>
#include <ListView.h>
#include <ScrollView.h>
#include <Window.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


class LibraryWin : public BWindow {
    void LibraryView();

public:
    // GUI elements
    BScrollView *scroll;
    BListView *list;

    LibraryWin();

    bool QuitRequested();

    void MessageReceived(BMessage *message);

    void LoadLibrary(BDirectory *dir);
};

#endif

