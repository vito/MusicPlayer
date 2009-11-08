#ifndef __LIBRARY_H
#define __LIBRARY_H

#include <Entry.h>


struct LibraryFile {
    entry_ref file;
    LibraryFile *next;
};


class Library {
    LibraryFile *head;
    int size;

public:
    Library() {}

    LibraryFile *At(int offset);

    void Add(entry_ref file);

    void Pop();

    int Count();
};

#endif
