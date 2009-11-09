all:
	g++ -O2 -Wall -o MusicPlayer MainWin.cpp LibraryItem.cpp LibraryWin.cpp Main.cpp -lbe -lmedia
