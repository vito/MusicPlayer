#include "Library.h"

void
Library::Add(BEntry file) {
    LibraryFile *entry = new LibraryFile;

    entry->file = file;
    entry->next = head;

    head = entry;

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

BEntry
Library::At(int offset) {
    LibraryFile cur = *head;

    for (int i = 1; i < (Count() - offset); i++)
        cur = *cur.next;

    return cur.file;
}

