#include "Library.h"

void
Library::Add(entry_ref file) {
    LibraryFile *lib = new LibraryFile;

    lib->file = file;
    lib->next = head;

    head = lib;

    size++;
}

void
Library::Pop() {
    head = head->next;
    size--;
}

int
Library::Count() {
    return size;
}

LibraryFile *
Library::At(int offset) {
    LibraryFile *cur = head;

    for (int i = 1; i < (Count() - offset); i++)
        cur = cur->next;

    return cur;
}

