#ifndef __LIBRARY_H
#define __LIBRARY_H

#include <Entry.h>


struct LibraryFile {
    BEntry file;
    LibraryFile *next;
};


class Library {
    LibraryFile *head;
    int size;

public:
    Library() {}

    BEntry At(int offset);

    void Add(BEntry file);

    void Pop();

    int Count();
};

#endif
