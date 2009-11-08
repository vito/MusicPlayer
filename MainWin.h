#ifndef __MAIN_WIN_H
#define __MAIN_WIN_H

#include <Button.h>
#include <StatusBar.h>
#include <Window.h>

const int32 UPDATE_PROGRESS = 'UPPR';


class MainWin : public BWindow {
    BButton *start;
    BButton *next;
    BButton *stop;
    BStatusBar *progress;

    void MainView();

public:
    MainWin();

    bool QuitRequested();

    void MessageReceived(BMessage *message);
};

#endif
