#include <Application.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>

#include "MainWin.h"

const int32 START = 'STRT';
const int32 STOP = 'STOP';
const int32 NEXT = 'NEXT';


MainWin::MainWin() :
    BWindow(BRect(0, 0, 1, 1), "Music Player", B_TITLED_WINDOW,
            B_ASYNCHRONOUS_CONTROLS | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS) {
    MainView();

    CenterOnScreen();
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
        case START:
        case STOP:
        case NEXT:
            be_app_messenger.SendMessage(message);
            break;
        default:
            BWindow::MessageReceived(message);
    }
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
}
