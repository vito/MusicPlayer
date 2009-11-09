#include "Main.h"


MusicPlayer::MusicPlayer() :
    BApplication("application/music-player") {}

void
MusicPlayer::ReadyToRun() {
    srand(time(NULL));

    main = new MainWin();
    main->Show();

    library = new LibraryWin();
    library->Show();
}


int main(void) {
    MusicPlayer app;
    app.Run();
    return 0;
}
