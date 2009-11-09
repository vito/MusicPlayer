#ifndef __MAIN_H
#define __MAIN_H

#include "MainWin.h"
#include "LibraryWin.h"

#include <Application.h>


class MusicPlayer : public BApplication {
public:
    MainWin *main;
    LibraryWin *library;

    MusicPlayer();

    void ReadyToRun();
};

#endif
