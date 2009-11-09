#ifndef __LIBRARY_ITEM_H
#define __LIBRARY_ITEM_H

#include <Entry.h>
#include <StringItem.h>


class LibraryItem : public BStringItem {
    char *fText;

public:
    entry_ref entry;

    LibraryItem(entry_ref file);
};

#endif
