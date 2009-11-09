#include "LibraryItem.h"

#include <stdio.h>


LibraryItem::LibraryItem(entry_ref file) :
    BStringItem(file.name) {
    printf("Constructing.\n");
    entry = file;
    printf("Constructed.\n");
}
