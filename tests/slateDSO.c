#include <stdlib.h>

#include "../include/slate.h"
#include "../include/slate_debug.h"

int dummy = 0;


struct SlDisplay *makeDisplay(void) {

    WARN();
    return slDisplay_create();
}

void destroyDisplay(struct SlDisplay *d) {

    WARN();
    slDisplay_destroy(d);
}




