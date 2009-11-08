#include <Application.h>

#include "MainWin.h"


class MusicPlayer : public BApplication {
    MainWin *win;

public:
    MusicPlayer() :
        BApplication("application/music-player") {}

    void ReadyToRun() {
        win = new MainWin();
        win->Show();
    }
};


int main(void) {
    MusicPlayer app;
    app.Run();
    return 0;
}
